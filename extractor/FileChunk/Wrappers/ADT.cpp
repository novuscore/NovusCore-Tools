#include "ADT.h"

#include <fstream>
#include <filesystem>

#include <Containers/StringTable.h>

#include "../../MPQ/MPQFileJobBatch.h"
#include "../../MAP/Chunk.h"
#include "../../BLP/BLP2PNG/BlpConvert.h"

namespace fs = std::filesystem;

void ADT::SaveToDisk(const std::string& fileName, MPQFileJobBatch* fileJobBatch)
{
    // We want to convert the ADT to a Chunk and save it to disk
    static Chunk* chunkTemplate = new Chunk(); // Don't change this one, we will use it in a memcpy to "reset" chunk
    static Chunk* chunk = new Chunk();
    static std::vector<AlphaMap> alphaMaps[MAP_CELLS_PER_CHUNK];

    memcpy(chunk, chunkTemplate, sizeof(Chunk));
    
    // The ADT should always have 256 cells
    assert(cells.size() == 256);

    // We need to get the name of all the textures this ADT uses
    std::vector<std::string> textureNames;

    Bytebuffer textureNameBuffer(mtex.filenames, mtex.size);

    while (textureNameBuffer.readData != textureNameBuffer.size)
    {
        std::string textureName;
        textureNameBuffer.GetString(textureName);
        textureNames.push_back(textureName);
    }

    // Create a StringTable for texture names
    StringTable stringTable;

    // Insert data from ADT into Chunk here
    for (u16 i = 0; i < MAP_CELLS_PER_CHUNK; i++)
    {
        Cell cell;
        cell.areaId = cells[i].mcnk.areaId;
        
        // Height data
        for (u16 j = 0; j < CELL_TOTAL_GRID_SIZE; j++)
        {
            f32 height = cells[i].mcvt.height[j] + cells[i].mcnk.position.z;

            cell.heightData[j] = height;

            if (height < chunk->heightHeader.gridMinHeight)
                chunk->heightHeader.gridMinHeight = height;
            if (height > chunk->heightHeader.gridMaxHeight)
                chunk->heightHeader.gridMaxHeight = height;
        }

        // Layers
        u32 numLayers = cells[i].mcnk.numLayers;

        for (u32 j = 0; j < numLayers; j++)
        {
            // Get the texture ID from the MCYL, this should be an index pointing into our textureNames vector
            u32 textureId = cells[i].mclys[j].textureId;

            // Get its name, add it to our StringTable
            const std::string& textureName = textureNames[textureId];

            fs::path texturePath = textureName;
            texturePath.replace_extension("dds");

            u32 stringTableIndex = stringTable.AddString(texturePath.string());

            // Store the stringTableIndex in the cell
            cell.layers[j].textureId = stringTableIndex;

            if (j > 0)
            {
                AlphaMap alphaMap;
                memcpy(&alphaMap.alphaMap, &cells[i].mcals[j-1].alphaMap, 4096);
                alphaMaps[i].push_back(alphaMap);
            }
        }

        // TODO: Possibly merge alphamaps like below, compress as DDS and append to .nmap?
        // Merge all layers alphamap into one array
        // Red channel maps to layer[1], Green channel maps to layer[2], Blue channel maps to layer[3]
        /*if (numLayers > 1)
        {
            
            for (u32 pixel = 0; pixel < 4096; pixel++)
            {
                for (u32 channel = 1; channel < numLayers; channel++)
                {
                    u32 dst = (pixel * 3) + (channel - 1);

                    cell.alphaMap.alphaMap[dst] = cells[i].mcals[channel - 1].alphaMap[pixel];
                }
            }
        }*/
        
        // TODO: Specular?

        chunk->cells[i] = cell;
    }

    // Extract all textures referenced by this ADT
    int textureIndex = 0;
    for(const std::string& textureName : textureNames)
    {
        bool loadSpecular = true;
        if (mtxfs.size() > textureIndex)
        {
            MTXF textureFlags = mtxfs[textureIndex++];

            // We don't load specular if the texture is a cubemap
            if (textureFlags.flags & static_cast<u32>(MTXFFlags::UseCubemap))
            {
                loadSpecular = false;
            }
        }
        
        // Extract diffuse texture
        fileJobBatch->AddFileJob(textureName, [textureName](std::shared_ptr<Bytebuffer> byteBuffer)
        {
            fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(textureName);
            outputPath = outputPath.make_preferred().replace_extension("dds");

            fs::create_directories(outputPath.parent_path());

            // Convert from BLP to DDS
            BLP::BlpConvert blpConvert;
            blpConvert.Convert(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string());
        });

        // Also extract specular if we should
        if (loadSpecular)
        {
            fs::path specularPath(textureName);

            std::string filename = specularPath.filename().stem().string();
            filename += "_s.blp";
            specularPath = specularPath.replace_filename(filename);

            fileJobBatch->AddFileJob(specularPath.string(), [specularPath](std::shared_ptr<Bytebuffer> byteBuffer)
            {
                fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(specularPath.string()).make_preferred();
                outputPath = outputPath.make_preferred().replace_extension("dds");

                fs::create_directories(outputPath.parent_path());

                BLP::BlpConvert blpConvert;
                blpConvert.Convert(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string());
            });
        }

        textureIndex++;
    }

    // Create a file
    std::ofstream output(fileName, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create map file. Check admin permissions\n");
        return;
    }

    // Write the Chunk to file
    //output.write(reinterpret_cast<char const*>(chunk), sizeof(Chunk)); // Old way of doing it, remove once we know we like it this way

    output.write(reinterpret_cast<char const*>(&chunk->chunkHeader), sizeof(chunk->chunkHeader));
    output.write(reinterpret_cast<char const*>(&chunk->heightHeader), sizeof(chunk->heightHeader));
    output.write(reinterpret_cast<char const*>(&chunk->heightBox), sizeof(chunk->heightBox));

    for (u32 i = 0; i < MAP_CELLS_PER_CHUNK; i++)
    {
        output.write(reinterpret_cast<char const*>(&chunk->cells[i]), sizeof(chunk->cells[i])); // Write cell

        u32 numAlphaMaps = static_cast<u32>(alphaMaps[i].size());
        output.write(reinterpret_cast<char const*>(&numAlphaMaps), sizeof(u32)); // Write number of alpha maps
        if (numAlphaMaps > 0)
        {
            output.write(reinterpret_cast<char const*>(alphaMaps[i].data()), sizeof(AlphaMap)* numAlphaMaps); // Write alpha maps
        }
        alphaMaps[i].clear();
    }

    /*for (Cell& cell : chunk->cells)
    {
        output.write(reinterpret_cast<char const*>(&cell.data), sizeof(cell.data)); // Write cell data

        u32 numAlphaMaps = static_cast<u32>(cell.alphaMaps.size());
        output.write(reinterpret_cast<char const*>(&numAlphaMaps), sizeof(u32)); // Write number of alpha maps
        if (numAlphaMaps > 0)
        {
            output.write(reinterpret_cast<char const*>(cell.alphaMaps.data()), sizeof(AlphaMap)* numAlphaMaps); // Write alpha maps
        }
    }*/

    // Serialize our StringTable and write it to the file
    std::shared_ptr<Bytebuffer> stringTableByteBuffer = Bytebuffer::Borrow<1048576>();
    stringTable.Serialize(*stringTableByteBuffer);
    output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);

    output.close();
}