/*
# MIT License

# Copyright(c) 2018-2019 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/
#pragma once
#include <Utils/DebugHandler.h>
#include <filesystem>
#include <sstream>
#include "../Utils/ServiceLocator.h"

#include "../FileChunk/ChunkLoader.h"
#include "../FileChunk/Wrappers/WDT.h"
#include "../FileChunk/Wrappers/ADT.h"

namespace MapLoader
{
    namespace fs = std::filesystem;
void LoadMaps(std::vector<std::string> internalMapNames)
{
    NC_LOG_MESSAGE("Extracting ADTs...");
    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<ChunkLoader> chunkLoader = ServiceLocator::GetChunkLoader();
    std::filesystem::path outputPath = fs::current_path().append("ExtractedData/Maps");

    for (const std::string& internalName : internalMapNames)
    {
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

        std::shared_ptr<ByteBuffer> fileWDT = mpqLoader->GetFile(filePathStream.str());
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

            ADT adt;
            for (u32 i = 0; i < NUM_SM_AREA_INFO; i++)
            {
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

                std::shared_ptr<ByteBuffer> fileADT = mpqLoader->GetFile(filePathStream.str());
                assert(fileADT); // If this file does not exist, something went very wrong

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

                // Extract data we want into our own format and then write adt to disk
            }
        }
        else
        {
            // Here we have a map with just a global map object
        }
    }

    return;
}
} // namespace MapLoader