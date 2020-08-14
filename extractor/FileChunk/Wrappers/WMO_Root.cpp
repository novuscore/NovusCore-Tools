#include "WMO_ROOT.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <tracy/Tracy.hpp>

#include <Containers/StringTable.h>
#include <Utils/StringUtils.h>

#include "../../MPQ/MPQLoader.h"
#include "../../BLP/BLP2PNG/BlpConvert.h"
#include "../../Utils/JobBatch.h"
#include "../../Utils/ServiceLocator.h"
#include "../../MAP/MapObjectRoot.h"

#include "../../FileChunk/Chunks/WMO/ROOT/MOMT.h"

namespace fs = std::filesystem;

std::string GetTextureNameByOffset(Bytebuffer& byteBuffer, size_t offset)
{
    while (offset < byteBuffer.size)
    {
        char c = byteBuffer.GetDataPointer()[offset];
        if (c != 0)
            break;

        offset++;
    }

    std::string result;
    while (offset < byteBuffer.size)
    {
        char c = byteBuffer.GetDataPointer()[offset];
        if (c == 0)
            break;

        result += c;
        offset++;
    }
    return result;
}

void WMO_ROOT::SaveToDisk(const std::string& fileName, StringTable& textureStringTable, JobBatch& jobBatch)
{
    ZoneScoped;

    // We want to convert the WMO_ROOT to a MapObjectRoot and save it to disk
    static MapObjectRoot* mapObjectRootTemplate = new MapObjectRoot(); // Don't change this one, we will use it in a memcpy to "reset" object
    thread_local MapObjectRoot* mapObjectRoot = new MapObjectRoot();

    memcpy(mapObjectRoot, mapObjectRootTemplate, sizeof(MapObjectRoot));

    Bytebuffer textureNameBuffer(motx.textureNames, motx.size);

    // Create a StringTable for texture names
    StringTable stringTable;

    std::string textureName;
    fs::path texturePath;

    for (MOMT::MOMTData& momtData : momt.data)
    {
        MapObjectMaterial& material = mapObjectRoot->materials.emplace_back();

        if ((momtData.textureOffset1 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset1] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset1);
            textureName = texturePath.string();
            texturePath.replace_extension(".dds");
            material.textureID[0] = stringTable.AddString(texturePath.string());

            u32 textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.size());

            // Extract texture
            {
                fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(textureName);
                outputPath = outputPath.make_preferred().replace_extension("dds");

                textureStringTable.AddString(outputPath.parent_path().string());

                jobBatch.AddJob(textureNameHash, [textureName, outputPath]()
                {
                    ZoneScopedN("WMO_ROOT::SaveToFile::Extract Texture");

                    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
                    std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(textureName);

                    if (byteBuffer == nullptr) // The bytebuffer return is nullptr if the file didn't exist
                        return;

                    // Convert from BLP to DDS
                    BLP::BlpConvert blpConvert;
                    blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
                });
            }
        }
        
        if ((momtData.textureOffset2 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset2] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset2);
            textureName = texturePath.string();
            texturePath.replace_extension(".dds");
            material.textureID[1] = stringTable.AddString(texturePath.string());

            {
                u32 textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.size());

                fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(textureName);
                outputPath = outputPath.make_preferred().replace_extension("dds");

                textureStringTable.AddString(outputPath.parent_path().string());

                // Extract texture
                jobBatch.AddJob(textureNameHash, [textureName, outputPath]()
                {
                    ZoneScopedN("WMO_ROOT::SaveToFile::Extract Texture");

                    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
                    std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(textureName);

                    // Convert from BLP to DDS
                    BLP::BlpConvert blpConvert;
                    blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
                });
            }
        }

        if ((momtData.textureOffset3 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset3] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset3);
            textureName = texturePath.string();
            texturePath.replace_extension(".dds");
            material.textureID[2] = stringTable.AddString(texturePath.string());

            {
                u32 textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.size());

                fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(textureName);
                outputPath = outputPath.make_preferred().replace_extension("dds");

                textureStringTable.AddString(outputPath.parent_path().string());

                // Extract texture
                jobBatch.AddJob(textureNameHash, [textureName, outputPath]()
                {
                    ZoneScopedN("WMO_ROOT::SaveToFile::Extract Texture");

                    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
                    std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(textureName);

                    if (byteBuffer == nullptr) // The bytebuffer return is nullptr if the file didn't exist
                        return;

                    // Convert from BLP to DDS
                    BLP::BlpConvert blpConvert;
                    blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
                });
            }
        }
    }

    // Create a file
    std::ofstream output(fileName, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create MapObjectRoot file. Check admin permissions\n");
        return;
    }

    // Write header
    output.write(reinterpret_cast<char const*>(&mapObjectRoot->header), sizeof(mapObjectRoot->header));

    // Write number of materials
    u32 numMaterials = static_cast<u32>(mapObjectRoot->materials.size());
    output.write(reinterpret_cast<char const*>(&numMaterials), sizeof(u32));
    
    // Write materials
    if (numMaterials > 0)
    {
        output.write(reinterpret_cast<char const*>(mapObjectRoot->materials.data()), sizeof(MapObjectMaterial) * numMaterials);
    }

    // Write numGroups
    u32 numGroups = mohd.groupsNum;
    output.write(reinterpret_cast<char const*>(&numGroups), sizeof(u32));
    
    // Serialize and write our StringTable to the file
    std::shared_ptr<Bytebuffer> stringTableByteBuffer = Bytebuffer::Borrow<1048576>();
    stringTable.Serialize(stringTableByteBuffer.get());
    output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);

    output.close();
}