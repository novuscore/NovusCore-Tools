#include "MapExtractor.h"
#include "../GlobalData.h"
#include "../Utils/MPQLoader.h"
#include "../Utils/ServiceLocator.h"
#include "../Utils/JobBatchRunner.h"

#include <Utils/StringUtils.h>
#include <filesystem>
#include <sstream>

#include "../Formats/MAP/MapHeader.h"
#include "../Formats/FileChunk/FileChunkLoader.h"
#include "../Formats/FileChunk/Chunks/WDT/Wdt.h"
#include "../Formats/FileChunk/Chunks/ADT/Adt.h"
#include "../Formats/FileChunk/Chunks/WMO/Root/MapObjectRoot.h"
#include "../Formats/FileChunk/Chunks/WMO/Object/MapObjectGroup.h"

#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void MapExtractor::ExtractMaps(std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    ZoneScoped;

    auto& globalData = ServiceLocator::GetGlobalData();

    json& mapConfig = globalData->config.GetJsonObjectByKey("Map");
    if (mapConfig["Extract"] == false)
        return;

    NC_LOG_MESSAGE("Extracting ADTs...");

    std::filesystem::path mapAlphaMapPath = globalData->texturePath / "ChunkAlphaMaps/Maps";
    std::filesystem::create_directories(mapAlphaMapPath);

    const std::vector<NDBC::Map>& maps = globalData->dbcExtractor->GetMaps();

    size_t mapNames = mapConfig["MapNames"].size();
    size_t numMaps = mapNames ? mapNames : maps.size();

    JobBatch mapJobBatch;
    std::vector<JobBatch> mapSubJobBatches;
    mapSubJobBatches.resize(numMaps);

    moodycamel::ConcurrentQueue<JobBatchToken> jobBatchTokens;

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<FileChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();

    for (size_t i = 0; i < numMaps; i++)
    {
        ZoneScoped;
        const std::string& internalName = mapNames ? mapConfig["MapNames"][i] : globalData->dbcExtractor->GetStringTableFromNDBC("Maps"_h).GetString(maps[i].internalName);

        // Only process map if a WDT file is found
        {
            std::stringstream filePathStream;

            // WDT File
            filePathStream << "world\\maps\\" << internalName << "\\" << internalName << ".WDT";

            if (!mpqLoader->HasFile(filePathStream.str()))
                continue;
        }

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

            Wdt::Wdt wdt;
            if (!chunkLoader->LoadWdt(fileWDT, wdt))
                return;

            MapHeader mapHeader;
            mapHeader.flags.UseMapObjectInsteadOfTerrain = wdt.mphd.flags.UsesGlobalMapObj;

            if (!wdt.mphd.flags.UsesGlobalMapObj)
            {
                filePathStream.clear();
                filePathStream.str("");

                JobBatch& batch = mapSubJobBatches[i];

                for (u32 j = 0; j < NUM_SM_AREA_INFO; j++)
                {
                    Wdt::Main::SMAreaInfo& areaInfo = wdt.main.MapAreaInfo[j];
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

                        Adt::Adt adt;
                        if (!chunkLoader->LoadAdt(fileADT, wdt, adt))
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
                            _wmoStringTable.AddString(wmoName);
                        }

                        std::filesystem::path adtSubPath = internalName;

                        // Extract data we want into our own format and then write adt to disk
                        adt.SaveToDisk(globalData, adtSubPath / (fileName + ".nchunk"), wdt);
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
                // Here we have a map with just a global map object (Save the WMO
                Bytebuffer wmoNameBuffer(wdt.mwmo.filenames, wdt.mwmo.size);

                std::string wmoPath;
                wmoNameBuffer.GetString(wmoPath);
                _wmoStringTable.AddString(wmoPath);

                fs::path nmorPath = wmoPath;
                nmorPath.replace_extension(".nmor");

                mapHeader.mapObjectName = nmorPath.string();

                Adt::Modf::ModfData& modf = wdt.modf.data[0];

                mapHeader.mapObjectPlacement.nameID = std::numeric_limits<u32>().max();
                mapHeader.mapObjectPlacement.position = modf.position;
                mapHeader.mapObjectPlacement.rotation = modf.rotation;
                mapHeader.mapObjectPlacement.scale = 1; // TODO: Until Legion this isn't used at all
            }

            // Add Map Header
            {
                filePathStream.clear();
                filePathStream.str("");

                filePathStream << internalName << "\\" << internalName << ".nmap";

                mapHeader.SaveToDisk(globalData, filePathStream.str());
            }
        });
    }

    size_t numMapJobCount = mapJobBatch.GetJobCount();
    NC_LOG_MESSAGE("Adding Map batch of %u jobs", numMapJobCount);

    if (numMapJobCount > 0)
    {
        JobBatchToken mainBatchToken = jobBatchRunner->AddBatch(mapJobBatch);
        mainBatchToken.WaitUntilFinished();
    }

    if (jobBatchTokens.size_approx() > 0)
    {
        JobBatchToken token;
        while (jobBatchTokens.try_dequeue(token))
        {
            token.WaitUntilFinished();
        }
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
            if (chunkLoader->LoadMapObjectRoot(fileWMORoot, wmoRoot))
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

                    if (chunkLoader->LoadMapObjectGroup(fileWMOObject, wmoRoot, wmoObject))
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

    size_t numWMOJobCount = wmoJobBatch.GetJobCount();
    NC_LOG_MESSAGE("Adding WMO batch of %u jobs", numWMOJobCount);

    if (numWMOJobCount > 0)
    {
        JobBatchToken wmoBatchToken = jobBatchRunner->AddBatch(wmoJobBatch);
        wmoBatchToken.WaitUntilFinished();
    }

    return;
}