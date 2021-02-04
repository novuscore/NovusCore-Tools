#pragma once
#include <NovusTypes.h>
#include "Chunk.h"
#include <fstream>

#pragma pack(push, 1)
struct MapDetailFlag
{
    u32 UseMapObjectInsteadOfTerrain : 1;
};

struct MapHeader
{
    u32 token = 1313685840; // UTF8 -> Binary -> Decimal for "nmap"
    u32 version = 2;

    MapDetailFlag flags;

    std::string mapObjectName = "";
    Placement mapObjectPlacement;

    void SaveToDisk(std::shared_ptr<GlobalData>& globalData, const fs::path& filePath)
    {
        // Create a file
        fs::path outputPath = (globalData->mapPath / filePath).make_preferred();
        std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
        if (!output)
        {
            DebugHandler::PrintError("Failed to create map file. Check admin permissions");
            return;
        }

        // Write the MapHeader to file
        output.write(reinterpret_cast<char const*>(&token), sizeof(token));
        output.write(reinterpret_cast<char const*>(&version), sizeof(version));
        output.write(reinterpret_cast<char const*>(&flags), sizeof(flags));

        // Write WMO Name
        if (flags.UseMapObjectInsteadOfTerrain)
        {
            output.write(mapObjectName.c_str(), mapObjectName.length() + 1);
            output.write(reinterpret_cast<char const*>(&mapObjectPlacement), sizeof(mapObjectPlacement));
        }

        output.close();
    }
};
#pragma pack(pop)