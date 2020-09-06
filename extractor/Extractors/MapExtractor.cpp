#include "MapExtractor.h"
#include "../GlobalData.h"
#include "../MPQ/MPQLoader.h"
#include "../Utils/ServiceLocator.h"
#include "../Utils/JobBatchRunner.h"

#include <Utils/StringUtils.h>
#include <filesystem>
#include <sstream>

#include "../FileChunk/ChunkLoader.h"
#include "../FileChunk/Wrappers/WDT.h"
#include "../FileChunk/Wrappers/ADT.h"
#include "../FileChunk/Wrappers/WMO_ROOT.h"
#include "../FileChunk/Wrappers/WMO_OBJECT.h"

#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void MapExtractor::ExtractMaps(std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    ZoneScoped;

    NC_LOG_MESSAGE("Extracting ADTs...");
    auto& globalData = ServiceLocator::GetGlobalData();

    json& mapConfig = globalData->config.GetJsonObjectByKey("Map");
    if (mapConfig["Extract"] == false)
        return;

    std::filesystem::path mapAlphaMapPath = globalData->texturePath / "ChunkAlphaMaps/Maps";
    std::filesystem::create_directories(mapAlphaMapPath);

    const std::vector<DBCMap>& maps = globalData->dbcExtractor->GetMaps();

    size_t mapNames = mapConfig["MapNames"].size();
    size_t numMaps = mapNames ? mapNames : maps.size();

    JobBatch mapJobBatch;
    std::vector<JobBatch> mapSubJobBatches;
    mapSubJobBatches.resize(numMaps);

    moodycamel::ConcurrentQueue<JobBatchToken> jobBatchTokens;

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<ChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();

    for (size_t i = 0; i < numMaps; i++)
    {
        ZoneScoped;
        const std::string& internalName = mapNames ? mapConfig["MapNames"][i] : globalData->dbcExtractor->GetStringTable().GetString(maps[i].InternalName);

        // Create Folders for the Map & Map's Alpha Map
        std::filesystem::create_directory(globalData->mapPath / internalName);
        std::filesystem::create_directory(mapAlphaMapPath / internalName);

        mapJobBatch.AddJob(0, [this, &globalData, &mpqLoader, &chunkLoader, &jobBatchRunner, &mapSubJobBatches, &jobBatchTokens, internalName, i]()
        {
            ZoneScopedN("MapLoader::Extract Maps");

            std::string fileName = "";
            std::stringstream fileNameStream;
            std::stringstream filePathStream;

            // WDT File
            filePathStream << "world\\maps\\" << internalName << "\\" << internalName << ".WDT";

            std::shared_ptr<Bytebuffer> fileWDT = mpqLoader->GetFile(filePathStream.str());
            if (!fileWDT)
                return;

            WDT wdt;
            if (!chunkLoader->LoadWDT(fileWDT, wdt))
                return;

            if ((wdt.mphd.flags & static_cast<u32>(MPHDFlags::UsesGlobalMapObj)) == 0)
            {
                filePathStream.clear();
                filePathStream.str("");

                JobBatch& batch = mapSubJobBatches[i];

                for (u32 j = 0; j < NUM_SM_AREA_INFO; j++)
                {
                    MAIN::SMAreaInfo& areaInfo = wdt.main.MapAreaInfo[j];
                    if (!areaInfo.hasADT)
                        continue;

                    u32 x = j % 64;
                    u32 y = j / 64;

                    fileNameStream.clear();
                    fileNameStream.str("");

                    filePathStream.clear();
                    filePathStream.str("");

                    fileNameStream << internalName << "_" << x << "_" << y;
                    fileName = fileNameStream.str();
                    filePathStream << "world\\maps\\" << internalName << "\\" << fileName << ".adt";

                    std::string filePath = filePathStream.str();

                    batch.AddJob(0, [this, &globalData, &mpqLoader, &chunkLoader, wdt, x, y, filePath, fileName, internalName]()
                    {
                        ZoneScoped;

                        std::shared_ptr<Bytebuffer> fileADT = mpqLoader->GetFile(filePath);
                        assert(fileADT); // If this file does not exist, something went very wrong

                        ADT adt;
                        if (!chunkLoader->LoadADT(fileADT, wdt, adt))
                        {
                            // This could happen, but for now I want to assert it in this test scenario
                            assert(false);
                        }

                        // Save all WMO names referenced by this ADT
                        Bytebuffer wmoNameBuffer(adt.mwmo.filenames, adt.mwmo.size);

                        while (wmoNameBuffer.readData != wmoNameBuffer.size)
                        {
                            std::string wmoName;
                            wmoNameBuffer.GetString(wmoName);

                            u32 index = _wmoStringTable.AddString(wmoName);
                        }

                        std::filesystem::path adtSubPath =  internalName;

                        // Extract data we want into our own format and then write adt to disk
                        adt.SaveToDisk(globalData, adtSubPath / (fileName + ".nmap"));
                    });
                }

                if (batch.GetJobCount())
                {
                    JobBatchToken token = jobBatchRunner->AddBatch(batch);
                    jobBatchTokens.enqueue(token);
                }
            }
            else
            {
                // Here we have a map with just a global map object
            }
        });
    }

    NC_LOG_MESSAGE("Adding Map batch of %u jobs", mapJobBatch.GetJobCount());

    JobBatchToken mainBatchToken = jobBatchRunner->AddBatch(mapJobBatch);
    mainBatchToken.WaitUntilFinished();

    JobBatchToken token;
    while (jobBatchTokens.try_dequeue(token))
    {
        token.WaitUntilFinished();
    }

    for (u32 i = 0; i < _wmoStringTable.GetNumStrings(); i++)
    {
        const std::string& wmoFilePath = _wmoStringTable.GetString(i);

        fs::path wmoPath = (globalData->wmoPath / wmoFilePath).make_preferred();
        fs::create_directories(wmoPath.parent_path());
    }

    // Extract WMOs
    JobBatch wmoJobBatch;

    for (u32 i = 0; i < _wmoStringTable.GetNumStrings(); i++)
    {
        const std::string& wmoFilePath = _wmoStringTable.GetString(i);

        wmoJobBatch.AddJob(0, [this, &globalData, &mpqLoader, &chunkLoader, &wmoFilePath, i]()
        {
            const std::string wmoBasePath = wmoFilePath.substr(0, wmoFilePath.length() - 4); // -3 removing (.wmo)
            const std::string wmoGroupBasePath = wmoBasePath + "_"; // adding (_)

            std::shared_ptr<Bytebuffer> fileWMORoot = mpqLoader->GetFile(wmoFilePath);
            WMO_ROOT wmoRoot;
            if (chunkLoader->LoadWMO_ROOT(fileWMORoot, wmoRoot))
            {
                std::stringstream ss;

                for (u32 i = 0; i < wmoRoot.mohd.groupsNum; i++)
                {
                    ss << wmoGroupBasePath << std::setw(3) << std::setfill('0') << i << ".wmo";

                    std::string wmoGroupFile = ss.str();

                    std::shared_ptr<Bytebuffer> fileWMOObject = mpqLoader->GetFile(wmoGroupFile);

                    // TODO: Add a safety check for GetFile (I've left it out now so we can ensure all WMO Group files exists)

                    WMO_OBJECT wmoObject;
                    wmoObject.root = &wmoRoot;

                    if (chunkLoader->LoadWMO_OBJECT(fileWMOObject, wmoRoot, wmoObject))
                    {
                        fs::path wmoGroupPathPath = (globalData->wmoPath / wmoGroupFile).make_preferred().replace_extension(".nmo");
                        wmoObject.SaveToDisk(wmoGroupPathPath, wmoRoot);
                    }

                    ss.clear();
                    ss.str("");
                }

                std::filesystem::path wmoRootPath = (globalData->wmoPath / (wmoBasePath + ".nmor")).make_preferred(); // .nmor
                wmoRoot.SaveToDisk(wmoRootPath);
            }
        });
    }

    NC_LOG_MESSAGE("Adding WMO batch of %u jobs", wmoJobBatch.GetJobCount());

    JobBatchToken wmoBatchToken = jobBatchRunner->AddBatch(wmoJobBatch);
    wmoBatchToken.WaitUntilFinished();

    return;
}