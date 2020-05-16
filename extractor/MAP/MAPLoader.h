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
#include "ADT.h"
#include "WDT.h"
#include "../MPQ/MPQHandler.h"
#include "../Utils/ServiceLocator.h"

namespace MapLoader
{
void LoadMaps(std::vector<std::string> adtLocationOutput)
{
    NC_LOG_MESSAGE("Extracting ADTs...");
    std::shared_ptr<MPQHandler> handler = ServiceLocator::GetMPQHandler();
    std::filesystem::path outputPath = ServiceLocator::GetMapFolderPath();

    MPQFile* file = nullptr;
    for (std::string adtName : adtLocationOutput)
    {
        bool createAdtDirectory = true;
        std::filesystem::path adtPath = outputPath.string() + "/" + adtName;
        if (std::filesystem::exists(adtPath))
        {
            // The reason we don't immediately create the folder is because there may not be any associated ADTs to the map (This can be solved by reading the WDL file)
            createAdtDirectory = false;
        }

        NC_LOG_MESSAGE("Extracting %s", adtName.c_str());

        std::string fileName = "";
        std::stringstream fileNameStream;
        std::stringstream filePathStream;

        // WDT File
        filePathStream << "world\\maps\\" << adtName << "\\" << adtName << ".WDT";

        std::shared_ptr<MPQFile> file = handler->GetFile(filePathStream.str());
        if (!file)
            continue;

        WDT mapWdt(file, adtName + ".wdt", adtPath.string());
        std::vector<u32> adtsToRead = mapWdt.Convert();

        filePathStream.clear();
        filePathStream.str("");

        for (u32 adt : adtsToRead)
        {
            u32 x = adt % 64;
            u32 y = adt / 64;

            fileNameStream.clear();
            fileNameStream.str("");

            filePathStream.clear();
            filePathStream.str("");

            fileNameStream << adtName << "_" << x << "_" << y;
            fileName = fileNameStream.str();
            filePathStream << "world\\maps\\" << adtName << "\\" << fileName << ".adt";

            std::shared_ptr<MPQFile> file = handler->GetFile(filePathStream.str());
            assert(file); // If this file does not exist, something went very wrong

            if (createAdtDirectory)
            {
                std::filesystem::create_directory(adtPath);
                createAdtDirectory = false;
            }

            ADT mapAdt(file, fileName + ".nmap", adtPath.string());
            mapAdt.Convert();
        }
    }

    return;
}
} // namespace MapLoader