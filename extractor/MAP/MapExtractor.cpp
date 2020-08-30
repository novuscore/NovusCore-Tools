#include "MapExtractor.h"
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

void MapExtractor::ExtractMaps(std::vector<std::string> internalMapNames, std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    ZoneScoped;

    NC_LOG_MESSAGE("Extracting ADTs...");
    std::filesystem::path outputPath = fs::current_path().append("ExtractedData");

    // Create base map folders
    std::filesystem::path mapFolderPath = outputPath.string() + "/Maps/";
    if (!std::filesystem::exists(mapFolderPath))
        std::filesystem::create_directory(mapFolderPath);

    std::filesystem::path mapAlphaMapFolderPath = outputPath.string() + "/Textures/ChunkAlphaMaps/Maps/";
    if (!std::filesystem::exists(mapAlphaMapFolderPath))
        std::filesystem::create_directories(mapAlphaMapFolderPath);

    JobBatch mapJobBatch;

    std::vector<JobBatch> mapSubJobBatches;
    mapSubJobBatches.resize(internalMapNames.size());

    moodycamel::ConcurrentQueue<JobBatchToken> jobBatchTokens;

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<ChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();

    for (size_t i = 0; i < internalMapNames.size(); i++)
    {
        ZoneScoped;
        const std::string& internalName = internalMapNames[i];
        mapJobBatch.AddJob(0, [this, &mpqLoader, &chunkLoader, &jobBatchRunner, &mapSubJobBatches, &jobBatchTokens, i, outputPath, internalName]()
        {
            ZoneScopedN("MapLoader::v::Extract Maps");

            //NC_LOG_MESSAGE("Extracting %s", internalName.c_str());

            std::filesystem::path alphaMapOutputFolderPath = outputPath.string() + "/Textures/ChunkAlphaMaps/Maps/" + internalName;
            if (!std::filesystem::exists(alphaMapOutputFolderPath))
                std::filesystem::create_directory(alphaMapOutputFolderPath);

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
            {
                return;
            }

            std::filesystem::path adtPath = outputPath.string() + "/Maps/" + internalName;
            if (!std::filesystem::exists(adtPath))
                std::filesystem::create_directory(adtPath);

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

                    batch.AddJob(0, [this, &mpqLoader, &chunkLoader, wdt, x, y, filePath, fileName, internalName]()
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

                        std::filesystem::path adtSubPath = "Maps/" + internalName;

                        // Extract data we want into our own format and then write adt to disk
                        adt.SaveToDisk(adtSubPath.string() + "/" + fileName + ".nmap", _textureFolderStringTable, _jobBatch);
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
        const std::string& wmoPath = outputPath.string() + "/MapObjects/" + _wmoStringTable.GetString(i);
        fs::path wmoFullPath = wmoPath;
        wmoFullPath = wmoFullPath.parent_path().make_preferred();

        fs::create_directories(wmoFullPath);
    }

    // Extract WMOs
    JobBatch wmoJobBatch;

    for (u32 i = 0; i < _wmoStringTable.GetNumStrings(); i++)
    {
        const std::string& wmoPath = _wmoStringTable.GetString(i);

        wmoJobBatch.AddJob(0, [this, &mpqLoader, &chunkLoader, &wmoPath, i, outputPath]()
        {
            const std::string wmoBasePath = wmoPath.substr(0, wmoPath.length() - 4); // -3 removing (.wmo)
            const std::string wmoGroupBasePath = wmoBasePath + "_"; // adding (_)

            std::shared_ptr<Bytebuffer> fileWMORoot = mpqLoader->GetFile(wmoPath);
            WMO_ROOT wmoRoot;
            if (chunkLoader->LoadWMO_ROOT(fileWMORoot, wmoRoot))
            {
                std::stringstream ss;

                for (u32 i = 0; i < wmoRoot.mohd.groupsNum; i++)
                {
                    ss << wmoGroupBasePath << std::setw(3) << std::setfill('0') << i << ".wmo";

                    std::string wmoGroupPath = ss.str();

                    std::shared_ptr<Bytebuffer> fileWMOObject = mpqLoader->GetFile(wmoGroupPath);

                    // TODO: Add a safety check for GetFile (I've left it out now so we can ensure all WMO Group files exists)

                    WMO_OBJECT wmoObject;
                    wmoObject.root = &wmoRoot;

                    if (chunkLoader->LoadWMO_OBJECT(fileWMOObject, wmoRoot, wmoObject))
                    {
                        fs::path wmoGroupPathPath = outputPath.string() + "/MapObjects/" + wmoGroupPath;
                        wmoGroupPathPath.replace_extension(".nmo"); // .nmo
                        wmoGroupPathPath.make_preferred();

                        wmoObject.SaveToDisk(wmoGroupPathPath.string(), wmoRoot,_jobBatch);
                    }

                    ss.clear();
                    ss.str("");
                }

                std::filesystem::path wmoRootPath = outputPath.string() + "/MapObjects/" + wmoBasePath + ".nmor"; // .nmor
                wmoRoot.SaveToDisk(wmoRootPath.string(), _textureFolderStringTable, _jobBatch);
            }
        });
    }

    NC_LOG_MESSAGE("Adding WMO batch of %u jobs", wmoJobBatch.GetJobCount());

    JobBatchToken wmoBatchToken = jobBatchRunner->AddBatch(wmoJobBatch);
    wmoBatchToken.WaitUntilFinished();

    // Create Directories for Textures
    for (u32 i = 0; i < _textureFolderStringTable.GetNumStrings(); i++)
    {
        const std::string& textureFolderPath = _textureFolderStringTable.GetString(i);
        fs::create_directories(textureFolderPath);
    }

    // Run file jobs
    _jobBatch.RemoveDuplicates();

    return;
}