#include "M2.h"
#include "../../GlobalData.h"
#include "../../Utils/MPQLoader.h"
#include "../../Extractors/TextureExtractor.h"

#include <fstream>
#include <Containers/StringTable.h>
#include <Utils/StringUtils.h>
#include "../../Utils/ServiceLocator.h"
#include "../../Utils/JobBatch.h"
#include "../BLP/BlpConvert.h"

#include <tracy/Tracy.hpp>

bool M2File::GetFromMPQ(std::string_view fileName)
{
    ZoneScopedN("M2::GetFromMPQ");

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<Bytebuffer> modelBuffer = mpqLoader->GetFile(fileName);

    if (!modelBuffer)
        return false;

    if (!modelBuffer->Get(m2))
        return false;

    m2Buffer = modelBuffer;

    const std::string m2BasePath = std::string(fileName.substr(0, fileName.length() - 3)) + "0"; // -3 removing (.m2)
    for (u32 i = 0; i < m2.numSkinProfiles; i++)
    {
        ZoneScopedN("M2::GetFromMPQ::GetSkinFile");

        const std::string m2SkinPath = m2BasePath + std::to_string(i) + ".skin";
        std::shared_ptr<Bytebuffer> skinBuffer = mpqLoader->GetFile(m2SkinPath);

        if (!skinBuffer)
            continue;

        M2SkinFile& skinFile =  skinFiles.emplace_back();
        if (!skinFile.GetFromBuffer(skinBuffer))
            return false;
    }

    return true;
 }

void M2File::SaveToDisk(const fs::path& filePath)
{
    ZoneScopedN("M2::SaveToFile");

    // Create a file
    std::ofstream output(filePath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create nm2 file. Check admin permissions\n");
        return;
    }

    // Write M2 Header
    output.write(reinterpret_cast<char const*>(&m2.header), sizeof(m2.header));

    // Write NumVertices & Vertices
    u32 numVertices = m2.vertices.size;
    output.write(reinterpret_cast<char const*>(&numVertices), sizeof(numVertices));
    if (numVertices > 0)
    {
        output.write(reinterpret_cast<char const*>(&m2Buffer->GetDataPointer()[m2.vertices.offset]), sizeof(M2Vertex) * numVertices);
    }

    // Write NumTextures & Textures
    u32 numTextures = m2.textures.size;
    output.write(reinterpret_cast<char const*>(&numTextures), sizeof(numTextures));

    if (numTextures > 0)
    {
        const StringTable& textureStringTable = ServiceLocator::GetGlobalData()->textureExtractor->GetStringTable();

        M2Texture* texture = nullptr;
        for (u32 i = 0; i < m2.textures.size; i++)
        {
            texture = m2.textures.GetElement(m2Buffer, i);

            output.write(reinterpret_cast<char const*>(&texture->type), sizeof(texture->type));
            output.write(reinterpret_cast<char const*>(&texture->flags), sizeof(texture->flags));

            u32 textureNameIndex = std::numeric_limits<u32>().max();

            if (texture->type == 0)
            {
                fs::path texturePath = texture->fileName.Get(m2Buffer);
                texturePath.replace_extension("dds");

                std::string texturePathStr = texturePath.string();
                std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

                u32 textureNameHash = StringUtils::fnv1a_32(texturePathStr.c_str(), texturePathStr.length());
                textureStringTable.TryFindHashedString(textureNameHash, textureNameIndex);
            }

            output.write(reinterpret_cast<char const*>(&textureNameIndex), sizeof(textureNameIndex));
        }
    }

    // Write NumMaterials & Materials
    u32 numMaterials = m2.materials.size;
    output.write(reinterpret_cast<char const*>(&numMaterials), sizeof(numMaterials));

    if (numMaterials > 0)
    {
        output.write(reinterpret_cast<char const*>(&m2Buffer->GetDataPointer()[m2.materials.offset]), sizeof(M2Material) * numMaterials);
    }

    // Write numTextureIndicesById & TextureIndicesById
    u32 numTextureIndicesById = m2.textureIndicesById.size;
    output.write(reinterpret_cast<char const*>(&numTextureIndicesById), sizeof(numTextureIndicesById));

    if (numTextureIndicesById > 0)
    {
        output.write(reinterpret_cast<char const*>(&m2Buffer->GetDataPointer()[m2.textureIndicesById.offset]), sizeof(u16) * numTextureIndicesById);
    }

    // Write numTextureCombos & TextureCombos
    u32 numTextureCombos = m2.textureCombos.size;
    output.write(reinterpret_cast<char const*>(&numTextureCombos), sizeof(numTextureCombos));

    if (numTextureCombos > 0)
    {
        output.write(reinterpret_cast<char const*>(&m2Buffer->GetDataPointer()[m2.textureCombos.offset]), sizeof(u16) * numTextureCombos);
    }

    // Write NumSkinProfiles
    u32 numSkinProfiles = m2.numSkinProfiles > 0;
    output.write(reinterpret_cast<char const*>(&numSkinProfiles), sizeof(numSkinProfiles));

    for (u32 i = 0; i < numSkinProfiles; i++)
    {
        M2SkinFile& skinFile = skinFiles[i];
        std::shared_ptr<Bytebuffer>& skinBuffer = skinFile.skinBuffer;

        M2Skin& skin = skinFile.skin;

        // Write Skin Header
        output.write(reinterpret_cast<char const*>(&skin.header), sizeof(skin.header));

        // Write NumSkinVertices & Vertices
        {
            u32 numSkinVertices = skin.vertices.size;
            output.write(reinterpret_cast<char const*>(&numSkinVertices), sizeof(numSkinVertices));

            if (numSkinVertices > 0)
            {
                output.write(reinterpret_cast<char const*>(&skinBuffer->GetDataPointer()[skin.vertices.offset]), sizeof(u16) * numSkinVertices);
            }
        }

        // Write NumSkinIndices & Indices
        {
            u32 numSkinIndices = skin.indices.size;
            output.write(reinterpret_cast<char const*>(&numSkinIndices), sizeof(numSkinIndices));

            if (numSkinIndices > 0)
            {
                output.write(reinterpret_cast<char const*>(&skinBuffer->GetDataPointer()[skin.indices.offset]), sizeof(u16) * numSkinIndices);
            }
        }

        u32 numSkinSubMeshes = skin.subMeshes.size;
        output.write(reinterpret_cast<char const*>(&numSkinSubMeshes), sizeof(numSkinSubMeshes));

        if (numSkinSubMeshes > 0)
        {
            M2SkinSelection* skinSelection = nullptr;
            M2Batch* skinBatch = nullptr;

            for (u32 j = 0; j < skin.subMeshes.size; j++)
            {
                skinSelection = skin.subMeshes.GetElement(skinBuffer, j);

                // Write SkinSelection
                {
                    u16 indexStart = skinSelection->indexStart;
                    u16 indexCount = skinSelection->indexCount;

                    output.write(reinterpret_cast<char const*>(&indexStart), sizeof(indexStart));
                    output.write(reinterpret_cast<char const*>(&indexCount), sizeof(indexCount));
                }
            }

            for (u32 j = 0; j < skin.subMeshes.size; j++)
            {
                skinBatch = skin.batches.GetElement(skinBuffer, j);

                // Write SkinBatch
                {
                    u8 flags = skinBatch->flags;
                    u16 shaderId = skinBatch->shaderId;
                    u16 skinSectionIndex = skinBatch->skinSectionIndex;
                    u16 geosetIndex = skinBatch->geosetIndex;
                    u16 materialIndex = skinBatch->materialIndex;
                    u16 textureCount = skinBatch->textureCount;
                    u16 textureComboIndex = skinBatch->textureComboIndex;

                    output.write(reinterpret_cast<char const*>(&flags), sizeof(flags));
                    output.write(reinterpret_cast<char const*>(&shaderId), sizeof(shaderId));
                    output.write(reinterpret_cast<char const*>(&skinSectionIndex), sizeof(skinSectionIndex));
                    output.write(reinterpret_cast<char const*>(&geosetIndex), sizeof(geosetIndex));
                    output.write(reinterpret_cast<char const*>(&materialIndex), sizeof(materialIndex));
                    output.write(reinterpret_cast<char const*>(&textureCount), sizeof(textureCount));
                    output.write(reinterpret_cast<char const*>(&textureComboIndex), sizeof(textureComboIndex));
                }
            }
        }
    }

    output.close();
}

bool M2SkinFile::GetFromBuffer(std::shared_ptr<Bytebuffer>& buffer)
{
    if (!buffer->Get(skin))
        return false;
    
    skinBuffer = buffer;
    return true;
}
