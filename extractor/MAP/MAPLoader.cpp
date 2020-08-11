#include "MAPLoader.h"
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

void MapLoader::LoadMaps(std::vector<std::string> internalMapNames, std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    ZoneScoped;

    NC_LOG_MESSAGE("Extracting ADTs...");
    std::filesystem::path outputPath = fs::current_path().append("ExtractedData");

    // Create base map folder
    std::filesystem::path mapFolderPath = outputPath.string() + "/Maps/";
    if (!std::filesystem::exists(mapFolderPath))
        std::filesystem::create_directory(mapFolderPath);

    JobBatch mapJobBatch;
    for (const std::string& internalName : internalMapNames)
    {
        ZoneScoped;

        mapJobBatch.AddJob(StringUtils::fnv1a_32(internalName.c_str(), internalName.size()), [this, outputPath, internalName]()
        {
            ZoneScopedN("MapLoader::v::Extract Maps");

            std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
            std::shared_ptr<ChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();

            //NC_LOG_MESSAGE("Extracting %s", internalName.c_str());

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
                // This could happen, but for now I want to assert it in this test scenario
                assert(false);
            }

            std::filesystem::path adtPath = outputPath.string() + "/Maps/" + internalName;
            if (!std::filesystem::exists(adtPath))
                std::filesystem::create_directory(adtPath);

            if ((wdt.mphd.flags & static_cast<u32>(MPHDFlags::UsesGlobalMapObj)) == 0)
            {
                filePathStream.clear();
                filePathStream.str("");

                for (u32 i = 0; i < NUM_SM_AREA_INFO; i++)
                {
                    ZoneScoped;

                    MAIN::SMAreaInfo& areaInfo = wdt.main.MapAreaInfo[i];
                    if (!areaInfo.hasADT)
                        continue;

                    u32 x = i % 64;
                    u32 y = i / 64;

                    fileNameStream.clear();
                    fileNameStream.str("");

                    filePathStream.clear();
                    filePathStream.str("");

                    fileNameStream << internalName << "_" << x << "_" << y;
                    fileName = fileNameStream.str();
                    filePathStream << "world\\maps\\" << internalName << "\\" << fileName << ".adt";

                    std::shared_ptr<Bytebuffer> fileADT = mpqLoader->GetFile(filePathStream.str());
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

                        u32 index = _stringTable.AddString(wmoName);
                    }

                    std::filesystem::path adtSubPath = "Maps/" + internalName;

                    // Extract data we want into our own format and then write adt to disk
                    adt.SaveToDisk(adtSubPath.string() + "/" + fileName + ".nmap", _jobBatch);
                }
            }
            else
            {
                // Here we have a map with just a global map object
            }
        });
    }

    NC_LOG_MESSAGE("Adding batch of %u jobs", mapJobBatch.GetJobCount());

    JobBatchToken token = jobBatchRunner->AddBatch(mapJobBatch);
    token.WaitUntilFinished();

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<ChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();
    // Extract WMOs
    for (u32 i = 0; i < _stringTable.GetNumStrings(); i++)
    {
        const std::string& wmoPath = _stringTable.GetString(i);
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

                    std::filesystem::create_directories(wmoGroupPathPath.parent_path());

                    wmoObject.SaveToDisk(wmoGroupPathPath.string(), _jobBatch);
                }

                ss.clear();
                ss.str("");
            }

            std::filesystem::path wmoRootPath = outputPath.string() + "/MapObjects/" + wmoBasePath + ".nmor"; // .nmor

            std::filesystem::create_directories(wmoRootPath.parent_path());

            wmoRoot.SaveToDisk(wmoRootPath.string(), _jobBatch);
        }
    }

    // Run file jobs
    _jobBatch.RemoveDuplicates();

    return;
}