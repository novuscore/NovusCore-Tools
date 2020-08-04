#include "MAPLoader.h"
#include "../MPQ/MPQLoader.h"
#include "../Utils/ServiceLocator.h"

#include <Containers/StringTable.h>
#include <filesystem>
#include <sstream>

#include "../FileChunk/ChunkLoader.h"
#include "../FileChunk/Wrappers/WDT.h"
#include "../FileChunk/Wrappers/ADT.h"
#include "../FileChunk/Wrappers/WMO_ROOT.h"
#include "../FileChunk/Wrappers/WMO_OBJECT.h"

#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void MapLoader::LoadMaps(std::vector<std::string> internalMapNames)
{
    ZoneScoped;

    NC_LOG_MESSAGE("Extracting ADTs...");
    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<ChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();
    std::filesystem::path outputPath = fs::current_path().append("ExtractedData/Maps");

    // Create a StringTable for WMO names
    StringTable stringTable;

    for (const std::string& internalName : internalMapNames)
    {
        ZoneScoped;

        bool createAdtDirectory = true;
        std::filesystem::path adtPath = outputPath.string() + "/" + internalName;
        if (std::filesystem::exists(adtPath))
        {
            // The reason we don't immediately create the folder is because there may not be any associated ADTs to the map (This can be solved by reading the WDL file)
            createAdtDirectory = false;
        }

        NC_LOG_MESSAGE("Extracting %s", internalName.c_str());

        std::string fileName = "";
        std::stringstream fileNameStream;
        std::stringstream filePathStream;

        // WDT File
        filePathStream << "world\\maps\\" << internalName << "\\" << internalName << ".WDT";

        std::shared_ptr<Bytebuffer> fileWDT = mpqLoader->GetFile(filePathStream.str());
        if (!fileWDT)
            continue;

        WDT wdt;
        if (!chunkLoader->LoadWDT(fileWDT, wdt))
        {
            // This could happen, but for now I want to assert it in this test scenario
            assert(false);
        }

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

                if (createAdtDirectory)
                {
                    std::filesystem::create_directory(adtPath);
                    createAdtDirectory = false;
                }

                // Save all WMO names referenced by this ADT
                Bytebuffer wmoNameBuffer(adt.mwmo.filenames, adt.mwmo.size);

                while (wmoNameBuffer.readData != wmoNameBuffer.size)
                {
                    std::string wmoName;
                    wmoNameBuffer.GetString(wmoName);

                    u32 index = stringTable.AddString(wmoName);
                }

                // Extract data we want into our own format and then write adt to disk
                adt.SaveToDisk(adtPath.string() + "/" + fileName + ".nmap", &_fileJobBatch);
            }
        }
        else
        {
            // Here we have a map with just a global map object
        }
    }

     _fileJobBatch.RemoveDuplicates();

    NC_LOG_MESSAGE("Running %u batched file jobs", _fileJobBatch.GetJobCount());
    _fileJobBatch.Process();

    // Extract WMOs
    for (u32 i = 0; i < stringTable.GetNumStrings(); i++)
    {
        const std::string& wmoName = stringTable.GetString(i);
        const std::string wmoGroupBaseName = wmoName.substr(0, wmoName.length() - 4) + "_"; // -3 removing (.wmo) and adding (_)

        std::shared_ptr<Bytebuffer> fileWMORoot = mpqLoader->GetFile(wmoName);
        WMO_ROOT wmoRoot;
        if (chunkLoader->LoadWMO_ROOT(fileWMORoot, wmoRoot))
        {
            std::stringstream ss;

            for (u32 i = 0; i < wmoRoot.mohd.groupsNum; i++)
            {
                ss << wmoGroupBaseName << std::setw(3) << std::setfill('0') << i << ".wmo";

                std::shared_ptr<Bytebuffer> fileWMOObject = mpqLoader->GetFile(ss.str());

                // TODO: Add a safety check for GetFile (I've left it out now so we can ensure all WMO Group files exists)

                WMO_OBJECT wmoObject;
                wmoObject.root = &wmoRoot;

                if (chunkLoader->LoadWMO_OBJECT(fileWMOObject, wmoRoot, wmoObject))
                {
                    // TODO: Implement SaveToDisk for WMO_OBJECT
                    //wmoObject.SaveToDisk();
                }

                ss.clear();
                ss.str("");
            }

            // TODO: Implement SaveToDisk for WMO_ROOT
            //wmoRoot.SaveToDisk();
        }
    }


    return;
}