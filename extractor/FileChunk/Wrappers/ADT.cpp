#include "ADT.h"

#include <fstream>
#include <filesystem>

#include <Utils/StringUtils.h>
#include <Utils/DebugHandler.h>

#include "../../MPQ/MPQLoader.h"
#include "../../Utils/ServiceLocator.h"
#include "../../Utils/JobBatch.h"
#include "../../MAP/Chunk.h"
#include "../../BLP/BLP2PNG/BlpConvert.h"

#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void ADT::SaveToDisk(const std::string& fileName, StringTable& textureFolderStringTable, JobBatch& jobBatch)
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

    // Create a StringTable for texture names
    StringTable stringTable;

    // Insert data from ADT into Chunk here
    for (u16 i = 0; i < MAP_CELLS_PER_CHUNK; i++)
    {
        Cell cell;
        cell.hole = cells[i].mcnk.holesLowRes;
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
        const u32 numLayers = cells[i].mcnk.numLayers;
        const u32 basePixelDestination = (i * CELL_ALPHAMAP_SIZE * MAP_CHUNK_ALPHAMAP_NUM_CHANNELS);

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
                    chunk->alphaMapData[dst] = cells[i].mcals[j - 1].alphaMap[pixel];
                }
            }
        }
        
        // TODO: Specular?

        chunk->cells[i] = cell;
    }

    // Extract all textures referenced by this ADT
    int textureIndex = 0;
    for(const std::string& textureName : textureNames)
    {
        bool loadSpecular = true;
        if (mtxf.data.size() > textureIndex)
        {
            const MTXF::MTXFData& textureFlags = mtxf.data[textureIndex++];

            // We don't load specular if the texture is a cubemap
            if (textureFlags.flags & static_cast<u32>(MTXFFlags::UseCubemap))
            {
                loadSpecular = false;
            }
        }
        
        // Extract diffuse texture
        {
            u32 textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.size());

            fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(textureName);
            outputPath = outputPath.make_preferred().replace_extension("dds");

            textureFolderStringTable.AddString(outputPath.parent_path().string());

            jobBatch.AddJob(textureNameHash, [textureName, outputPath]()
            {
                ZoneScopedN("ADT::SaveToFile::Extract Diffuse Texture");

                std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
                std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(textureName);

                if (byteBuffer == nullptr) // The bytebuffer return is nullptr if the file didn't exist
                    return;

                // Convert from BLP to DDS
                BLP::BlpConvert blpConvert;
                blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
            });
        }
        
        // Also extract specular if we should
        if (loadSpecular)
        {
            fs::path specularPath(textureName);

            std::string filename = specularPath.filename().stem().string();
            filename += "_s.blp";
            specularPath = specularPath.replace_filename(filename);

            {
                std::string specularName = specularPath.string();
                u32 textureNameHash = StringUtils::fnv1a_32(specularName.c_str(), specularName.size());

                fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(specularName).make_preferred();
                outputPath = outputPath.make_preferred().replace_extension("dds");

                textureFolderStringTable.AddString(outputPath.parent_path().string());

                jobBatch.AddJob(textureNameHash, [specularName, outputPath]()
                {
                    ZoneScopedN("ADT::SaveToFile::Extract Specular Texture");

                    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
                    std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(specularName);

                    if (byteBuffer == nullptr) // The bytebuffer return is nullptr if the file didn't exist
                        return;

                    BLP::BlpConvert blpConvert;
                    blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
                });
            }
            
        }

        textureIndex++;
    }

    // We need to get the name of all the WMOs this ADT uses
    if (modf.data.size() > 0)
    {
        Bytebuffer mwmoBuffer(mwmo.filenames, mwmo.size);
        Bytebuffer mwidBuffer(mwid.offset, mwid.size);

        // Insert WMO placement data into Chunk
        for (MODF::MODFData& modfData : modf.data)
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
    fs::path alphaMapSubPath = "Textures/ChunkAlphaMaps";
    alphaMapSubPath = alphaMapSubPath.append(fileName).make_preferred().replace_extension("dds");

    fs::path alphaMapOutputPath = fs::current_path().append("ExtractedData") / alphaMapSubPath;
    u32 alphaMapOutputPathHash = StringUtils::fnv1a_32(alphaMapOutputPath.string().c_str(), alphaMapOutputPath.string().size());

    //jobBatch.AddJob(alphaMapPathHash, [alphaMapPath, fileName, chunk->alphaMapData]()
    {
        ZoneScopedN("ADT::SaveToFile::Convert Alphamap Texture");
        
        BLP::BlpConvert blpConvert;
        blpConvert.ConvertRaw(64, 64, MAP_CELLS_PER_CHUNK, chunk->alphaMapData, MAP_CHUNK_ALPHAMAP_BYTE_SIZE, BLP::InputFormat::BGRA_8UB, BLP::Format::DXT1, alphaMapOutputPath.string(), false);
    }//);

    fs::path alphaMapDataPath = ("Data/extracted" / alphaMapSubPath).make_preferred();
    u32 alphaMapStringIndex = stringTable.AddString(alphaMapDataPath.string());
    
    // Create a file
    fs::path outputPath = fs::current_path().append("ExtractedData").append(fileName).make_preferred();
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create map file. Check admin permissions\n");
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
    stringTable.Serialize(*stringTableByteBuffer);
    output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);

    output.close();
}