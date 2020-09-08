#include "ADT.h"

#include <Utils/StringUtils.h>
#include <Utils/DebugHandler.h>

#include "../../../../GlobalData.h"
#include "../../../../Utils/ServiceLocator.h"
#include "../../../../Utils/JobBatch.h"
#include "../../../BLP/BlpConvert.h"
#include "../../../MAP/Chunk.h"

#include "../../../../Extractors/TextureExtractor.h"

#include <fstream>
#include <tracy/Tracy.hpp>

namespace Adt
{
    void Adt::SaveToDisk(std::shared_ptr<GlobalData>& globalData, const fs::path& filePath)
    {
        ZoneScoped;

        // We want to convert the ADT to a Chunk and save it to disk
        static Chunk* chunkTemplate = new Chunk(); // Don't change this one, we will use it in a memcpy to "reset" chunk
        thread_local Chunk* chunk = new Chunk();

        memcpy(chunk, chunkTemplate, sizeof(Chunk));

        // The ADT should always have 256 cells
        assert(cells.size() == 256);

        // We need to get the name of all the textures this ADT uses
        std::vector<std::string> textureNames;

        Bytebuffer textureNameBuffer(mtex.filenames, mtex.size);

        while (textureNameBuffer.readData != textureNameBuffer.size)
        {
            std::string& textureName = textureNames.emplace_back();
            textureNameBuffer.GetString(textureName);
        }

        // Create a StringTable for alpha map texture names
        StringTable stringTable;

        const StringTable& textureStringTable = ServiceLocator::GetGlobalData()->textureExtractor->GetStringTable();

        // Insert data from ADT into Chunk here
        for (u16 i = 0; i < MAP_CELLS_PER_CHUNK; i++)
        {
            Cell cell;
            cell.hole = cells[i].mcnk.holesLowRes;
            cell.areaId = cells[i].mcnk.areaId;

            // Per vertex in the cell
            for (u16 j = 0; j < CELL_TOTAL_GRID_SIZE; j++)
            {
                // Height data
                f32 height = cells[i].mcvt.height[j] + cells[i].mcnk.position.z;

                cell.heightData[j] = height;

                if (height < chunk->heightHeader.gridMinHeight)
                    chunk->heightHeader.gridMinHeight = height;
                if (height > chunk->heightHeader.gridMaxHeight)
                    chunk->heightHeader.gridMaxHeight = height;

                // Normal data, this is swizzled read https://wowdev.wiki/ADT/v18#MCNR_sub-chunk
                i8 normalX = cells[i].mcnr.entries[j].normal[0];
                i8 normalY = cells[i].mcnr.entries[j].normal[2];
                i8 normalZ = cells[i].mcnr.entries[j].normal[1];

                // Make sure to convert to u8 [0 .. 256]
                cell.normalData[j][0] = normalX + 127;
                cell.normalData[j][1] = normalY + 127;
                cell.normalData[j][2] = normalZ + 127;

                // Color data
                cell.colorData[j][0] = cells[i].mccv.entries[j].blue;
                cell.colorData[j][1] = cells[i].mccv.entries[j].green;
                cell.colorData[j][2] = cells[i].mccv.entries[j].red;
            }

            // Layers
            const u32 numLayers = cells[i].mcnk.numLayers;
            const u32 basePixelDestination = (i * CELL_ALPHAMAP_SIZE * MAP_CHUNK_ALPHAMAP_NUM_CHANNELS);

            for (u32 j = 0; j < numLayers; j++)
            {
                // Get the texture ID from the MCYL, this should be an index pointing into our textureNames vector
                u32 textureId = cells[i].mcly.data[j].textureId;

                // Get its name, add it to our StringTable
                const std::string& textureName = textureNames[textureId];

                fs::path texturePath = textureName;
                texturePath.replace_extension("dds");

                std::string texturePathStr = texturePath.string();
                std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

                u32 textureNameIndex = std::numeric_limits<u32>().max();

                u32 textureNameHash = StringUtils::fnv1a_32(texturePathStr.c_str(), texturePathStr.length());
                textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);

                // Store the stringTableIndex in the cell
                cell.layers[j].textureId = textureNameIndex;

                // If the layer has alpha data, add it to our per-chunk alphamap
                if (j > 0)
                {
                    // 0 = r, 1 = g, 2 = b, 3 = a
                    u32 swizzleMap[4] =
                    {
                        2,1,0,3
                    };
                    u32 channel = swizzleMap[j - 1];

                    for (u32 pixel = 0; pixel < CELL_ALPHAMAP_SIZE; pixel++)
                    {
                        u32 dst = basePixelDestination + (pixel * MAP_CHUNK_ALPHAMAP_NUM_CHANNELS) + channel;
                        chunk->alphaMapData[dst] = cells[i].mcal.data[j - 1].alphaMap[pixel];
                    }
                }
            }

            // TODO: Specular?

            chunk->cells[i] = cell;
        }

        // We need to get the name of all the WMOs this ADT uses
        if (modf.data.size() > 0)
        {
            Bytebuffer mwmoBuffer(mwmo.filenames, mwmo.size);
            Bytebuffer mwidBuffer(mwid.offset, mwid.size);

            // Insert WMO placement data into Chunk
            for (Modf::ModfData& modfData : modf.data)
            {
                MapObjectPlacement& mapObjectPlacement = chunk->mapObjectPlacements.emplace_back();

                u32 offsetIntoMWMO;
                if (!mwidBuffer.Get<u32>(offsetIntoMWMO, modfData.nameId * sizeof(u32)))
                {
                    NC_LOG_FATAL("Could not get MWMO Offset from MWID");
                }

                std::string wmoName;
                mwmoBuffer.GetStringByOffset(wmoName, offsetIntoMWMO);

                if (wmoName.empty())
                {
                    NC_LOG_FATAL("Could not get WMO Name from MWMO");
                }

                fs::path nmorPath = wmoName;
                nmorPath.replace_extension(".nmor");

                mapObjectPlacement.nameID = stringTable.AddString(nmorPath.string());
                mapObjectPlacement.position = modfData.position;
                mapObjectPlacement.rotation = modfData.rotation;
                mapObjectPlacement.scale = 1; // TODO: Until Legion this isn't used at all
            }
        }

        // Convert alphamap data into a texture
        json& mapConfig = globalData->config.GetJsonObjectByKey("Map");
        if (mapConfig["ExtractChunkAlphaMaps"] == true)
        {
            ZoneScopedN("ADT::SaveToFile::Convert Alphamap Texture");

            const std::string alphaMapOutput = ((globalData->texturePath / "ChunkAlphaMaps/Maps" / filePath).make_preferred().replace_extension("dds")).string();

            BLP::BlpConvert blpConvert;
            blpConvert.ConvertRaw(64, 64, MAP_CELLS_PER_CHUNK, chunk->alphaMapData, MAP_CHUNK_ALPHAMAP_BYTE_SIZE, BLP::InputFormat::BGRA_8UB, BLP::Format::DXT1, alphaMapOutput, false);
        }

        const fs::path alphaMapRelativePath = ("Textures/ChunkAlphaMaps/Maps" / filePath).make_preferred().replace_extension("dds");
        u32 alphaMapStringIndex = stringTable.AddString(alphaMapRelativePath.string());

        // Create a file
        fs::path outputPath = (globalData->mapPath / filePath).make_preferred();
        std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
        if (!output)
        {
            NC_LOG_ERROR("Failed to create map file. Check admin permissions");
            return;
        }

        // Write the Chunk to file
        output.write(reinterpret_cast<char const*>(&chunk->header), sizeof(chunk->header));
        output.write(reinterpret_cast<char const*>(&chunk->heightHeader), sizeof(chunk->heightHeader));
        output.write(reinterpret_cast<char const*>(&chunk->heightBox), sizeof(chunk->heightBox));

        output.write(reinterpret_cast<char const*>(&chunk->cells[0]), sizeof(Cell) * MAP_CELLS_PER_CHUNK);

        output.write(reinterpret_cast<char const*>(&alphaMapStringIndex), sizeof(u32)); // Write alpha map string index

        u32 numMapObjectPlacements = static_cast<u32>(chunk->mapObjectPlacements.size());
        output.write(reinterpret_cast<char const*>(&numMapObjectPlacements), sizeof(u32)); // Write number of map object placements

        if (numMapObjectPlacements > 0)
        {
            output.write(reinterpret_cast<char const*>(chunk->mapObjectPlacements.data()), sizeof(MapObjectPlacement) * numMapObjectPlacements); // Write map object placements
        }

        // Serialize our StringTable and write it to the file
        std::shared_ptr<Bytebuffer> stringTableByteBuffer = Bytebuffer::Borrow<1048576>();
        stringTable.Serialize(stringTableByteBuffer.get());
        output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);

        output.close();
    }
}