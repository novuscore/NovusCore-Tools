#include "MapObjectRoot.h"

#include <Utils/StringUtils.h>
#include <Containers/StringTable.h>

#include "../../../../../GlobalData.h"
#include "../../../../../Utils/JobBatch.h"
#include "../../../../../Utils/MPQLoader.h"
#include "../../../../../Utils/ServiceLocator.h"
#include "../../../../BLP/BlpConvert.h"

#include "../../../../MAP/MapObjectRoot.h"
#include "../../WMO/ROOT/MOMT.h"

#include "../../../../../Extractors/TextureExtractor.h"

#include <fstream>
#include <tracy/Tracy.hpp>


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

void WMO_ROOT::SaveToDisk(const fs::path& filePath)
{
    ZoneScoped;

    // We want to convert the WMO_ROOT to a MapObjectRoot and save it to disk
    static MapObjectRoot* mapObjectRootTemplate = new MapObjectRoot(); // Don't change this one, we will use it in a memcpy to "reset" object
    thread_local MapObjectRoot* mapObjectRoot = new MapObjectRoot();

    memcpy(mapObjectRoot, mapObjectRootTemplate, sizeof(MapObjectRoot));

    Bytebuffer textureNameBuffer(motx.textureNames, motx.size);

    const StringTable& textureStringTable = ServiceLocator::GetGlobalData()->textureExtractor->GetStringTable();

    std::string textureName;
    fs::path texturePath;
    u32 textureNameIndex = 0;
    u32 textureNameHash = 0;

    for (MOMT::MOMTData& momtData : momt.data)
    {
        MapObjectMaterial& material = mapObjectRoot->materials.emplace_back();
        material.materialType = momtData.shader;
        material.transparencyMode = momtData.blendMode;
        material.flags = momtData.flags;

        if ((momtData.textureOffset1 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset1] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset1);
            texturePath.replace_extension("dds");

            textureName = texturePath.string();
            std::transform(textureName.begin(), textureName.end(), textureName.begin(), ::tolower);

            textureNameIndex = std::numeric_limits<u32>().max();
            textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.length());

            textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);
            material.textureID[0] = textureNameIndex;
        }
        
        if ((momtData.textureOffset2 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset2] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset2);
            texturePath.replace_extension("dds");

            textureName = texturePath.string();
            std::transform(textureName.begin(), textureName.end(), textureName.begin(), ::tolower);

            textureNameIndex = std::numeric_limits<u32>().max();
            textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.length());

            textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);
            material.textureID[1] = textureNameIndex;
        }

        if ((momtData.textureOffset3 < textureNameBuffer.size) && (textureNameBuffer.GetDataPointer()[momtData.textureOffset3] != '\0'))
        {
            texturePath = GetTextureNameByOffset(textureNameBuffer, momtData.textureOffset3);
            texturePath.replace_extension("dds");

            textureName = texturePath.string();
            std::transform(textureName.begin(), textureName.end(), textureName.begin(), ::tolower);

            textureNameIndex = std::numeric_limits<u32>().max();
            textureNameHash = StringUtils::fnv1a_32(textureName.c_str(), textureName.length());

            textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);
            material.textureID[2] = textureNameIndex;
        }
    }

    // Create a file
    std::ofstream output(filePath, std::ofstream::out | std::ofstream::binary);
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

    output.close();

    mapObjectRoot->materials.clear();
}