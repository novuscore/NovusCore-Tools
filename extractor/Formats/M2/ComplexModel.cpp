#include "ComplexModel.h"
#include "M2.h"
#include "../../GlobalData.h"
#include "../../Utils/ServiceLocator.h"
#include "../../Utils/NormalCompression.h"

#include <fstream>
#include <tracy/Tracy.hpp>
#include <Utils/StringUtils.h>

void ComplexModel::ReadFromM2(M2File& file)
{
    M2& m2 = file.m2;
    M2Skin& skin = file.skin;
    
    name = m2.name.Get(file.m2Buffer);
    flags = *reinterpret_cast<ComplexModelFlag*>(&m2.flags);

    // Read Sequences
    {
        u32 numSequences = m2.sequences.size;
        u32 sequencesOffset = m2.sequences.offset;

        size_t existingSequences = sequences.size();
        sequences.resize(numSequences);
        for (u32 i = 0; i < numSequences; i++)
        {
            M2Sequence* m2Sequence = m2.sequences.GetElement(file.m2Buffer, i);

            ComplexAnimationSequence& sequence = sequences[existingSequences + i];
            sequence.id = m2Sequence->id;
            sequence.subId = m2Sequence->variationId;

            sequence.duration = m2Sequence->duration;
            sequence.moveSpeed = m2Sequence->moveSpeed;

            sequence.flags.isAlwaysPlaying = false;
            sequence.flags.isAlias = m2Sequence->flags.isAlias;
            sequence.flags.blendTransition = m2Sequence->flags.blendTransition;

            sequence.frequency = m2Sequence->frequency;
            sequence.repetitionRange = uvec2(m2Sequence->repeatRange.minimum, m2Sequence->repeatRange.maximum);
            sequence.blendTimeStart = m2Sequence->blendTime;
            sequence.blendTimeEnd = m2Sequence->blendTime;

            sequence.extentsMin = m2Sequence->bounds.extent.min;
            sequence.extentsMax = m2Sequence->bounds.extent.max;
            sequence.radius = m2Sequence->bounds.radius;

            sequence.nextVariationId = m2Sequence->nextVariationId;
            sequence.nextAliasId = m2Sequence->nextAliasId;
        }
    }

    // Read Bones
    {
        u32 numBones = m2.bones.size;
        u32 bonesOffset = m2.bones.offset;

        bones.resize(numBones);
        for (u32 i = 0; i < numBones; i++)
        {
            M2CompBone* m2Bone = m2.bones.GetElement(file.m2Buffer, i);

            ComplexBone& bone = bones[i];
            bone.primaryBoneIndex = m2Bone->keyBoneId;
            
            bone.flags.ignoreParentTranslate = m2Bone->flags.ignoreParentTranslate;
            bone.flags.ignoreParentScale = m2Bone->flags.ignoreParentScale;
            bone.flags.ignoreParentRotation = m2Bone->flags.ignoreParentRotation;
            bone.flags.sphericalBillboard = m2Bone->flags.sphericalBillboard;
            bone.flags.cylindricalBillboard_LockX = m2Bone->flags.cylindricialBillboardLockX;
            bone.flags.cylindricalBillboard_LockY = m2Bone->flags.cylindricialBillboardLockY;
            bone.flags.cylindricalBillboard_LockZ = m2Bone->flags.cylindricialBillboardLockZ;
            bone.flags.unk_0x80 = m2Bone->flags.unk_0x80;
            bone.flags.transformed = m2Bone->flags.transformed;
            bone.flags.kinematicBone = m2Bone->flags.kinematicBone;
            bone.flags.helmetAnimationScale = m2Bone->flags.helmetAnimScaled;
            bone.flags.unk_0x1000 = m2Bone->flags.sequenceId;

            bone.parentBoneId = m2Bone->parentBone;
            bone.submeshId = m2Bone->submeshId;

            // Read Translation Track
            {
                FillAnimationTrackFromM2Track(file, bone.translation, m2Bone->translation);
            }

            // Read Rotation Track
            {
                u32 numTracks = m2Bone->rotation.values.size;

                bone.rotation.interpolationType = AnimationTrackInterpolationType::LINEAR;
                bone.rotation.tracks.reserve(numTracks);

                for (u32 i = 0; i < numTracks; i++)
                {
                    M2Array<u32>* m2Timestamps = m2Bone->rotation.timestamps.GetElement(file.m2Buffer, i);
                    M2Array<M2CompQuat>* m2Values = m2Bone->rotation.values.GetElement(file.m2Buffer, i);

                    if (m2Timestamps->size > 0 && m2Values->size > 0)
                    {
                        ComplexAnimationTrack<vec4>& track = bone.rotation.tracks.emplace_back();

                        track.sequenceId = i;

                        track.timestamps.resize(m2Timestamps->size);
                        memcpy(track.timestamps.data(), m2Timestamps->Get(file.m2Buffer), sizeof(u32) * m2Timestamps->size);

                        track.values.resize(m2Values->size);
                        for (u32 j = 0; j < m2Values->size; j++)
                        {
                            track.values[j] = m2Values->GetElement(file.m2Buffer, j)->ToVec4();
                        }
                    }
                }
            }

            // Read Scale Track
            {
                FillAnimationTrackFromM2Track(file, bone.scale, m2Bone->scale);
            }

            bone.pivot = m2Bone->pivot;
        }
    }

    // Read Vertices
    {
        u32 numVertices = m2.vertices.size;
        vertices.resize(numVertices);

        for (u32 i = 0; i < numVertices; i++)
        {
            M2Vertex* m2Vertex = m2.vertices.GetElement(file.m2Buffer, i);

            ComplexVertex& vertex = vertices[i];
            vertex.position = m2Vertex->position;

            vertex.uvCords[0] = m2Vertex->uvCords[0];
            vertex.uvCords[1] = m2Vertex->uvCords[1];

            vec3 normal = glm::normalize(vec3(-m2Vertex->normal.x, -m2Vertex->normal.y, m2Vertex->normal.z));
            vec2 octNormal = Utils::OctNormalEncode(normal);
            vertex.octNormal[0] = static_cast<u8>(octNormal.x * 255.0f);
            vertex.octNormal[1] = static_cast<u8>(octNormal.y * 255.0f);

            for (u32 j = 0; j < 4; j++)
            {
               vertex.boneIndices[j] = m2Vertex->boneIndices[j];
               vertex.boneWeights[j] = m2Vertex->boneWeights[j];
            }

            vec3 transformedPosition = vec3(-vertex.position.x, -vertex.position.y, vertex.position.z);
            for (u32 j = 0; j < 3; j++)
            {
                if (transformedPosition[j] < cullingData.minBoundingBox[j])
                    cullingData.minBoundingBox[j] = transformedPosition[j];

                if (transformedPosition[j] > cullingData.maxBoundingBox[j])
                    cullingData.maxBoundingBox[j] = transformedPosition[j];
            }
        }

        cullingData.boundingSphereRadius = m2.boundingSphereRadius;
    }

    // Read Textures
    {
        u32 numTextures = m2.textures.size;
        u32 texturesOffset = m2.textures.offset;

        textures.resize(numTextures);
        for (u32 i = 0; i < numTextures; i++)
        {
            ComplexTexture& texture = textures[i];
            M2Texture* m2Texture = m2.textures.GetElement(file.m2Buffer, i);

            texture.type = static_cast<ComplexTextureType>(m2Texture->type);
            texture.flags = *reinterpret_cast<ComplexTextureFlag*>(&m2Texture->flags);
            texture.fileName = m2Texture->fileName.Get(file.m2Buffer);
        }
    }
    
    // Read Materials
    {
        u32 numMaterials = m2.materials.size;
        u32 materialsOffset = m2.materials.offset;

        materials.resize(numMaterials);
        memcpy(materials.data(), &file.m2Buffer->GetDataPointer()[materialsOffset], numMaterials * sizeof(ComplexMaterial));
    }

    // Read Texture Index Lookup Table
    {
        u32 numTextureLookupIds = m2.textureLookupTable.size;
        u32 textureLookupIdsOffset = m2.textureLookupTable.offset;

        textureIndexLookupTable.resize(numTextureLookupIds);
        memcpy(textureIndexLookupTable.data(), &file.m2Buffer->GetDataPointer()[textureLookupIdsOffset], numTextureLookupIds * sizeof(u16));
    }

    // Read Texture Unit Lookup Table
    {
        u32 numTextureUnitLookupIds = m2.textureUnitLookupTable.size;
        u32 textureUnitLookupIdsOffset = m2.textureUnitLookupTable.offset;

        textureUnitLookupTable.resize(numTextureUnitLookupIds);
        memcpy(textureUnitLookupTable.data(), &file.m2Buffer->GetDataPointer()[textureUnitLookupIdsOffset], numTextureUnitLookupIds * sizeof(u16));
    }

    // Read Texture Transparency Lookup Table
    {
        u32 numTextureTransparencyLookupIds = m2.textureTransparencyLookupTable.size;
        u32 textureTransparencyLookupIdsOffset = m2.textureTransparencyLookupTable.offset;

        textureTransparencyLookupTable.resize(numTextureTransparencyLookupIds);
        memcpy(textureTransparencyLookupTable.data(), &file.m2Buffer->GetDataPointer()[textureTransparencyLookupIdsOffset], numTextureTransparencyLookupIds * sizeof(u16));
    }

    // Read Texture UV Animation Lookup Table
    {
        u32 numTextureUVAnimationLookupIds = m2.textureUVAnimationLookup.size;
        u32 textureUVAnimationLookupIdsOffset = m2.textureUVAnimationLookup.offset;

        textureUVAnimationLookupTable.resize(numTextureUVAnimationLookupIds);
        memcpy(textureUVAnimationLookupTable.data(), &file.m2Buffer->GetDataPointer()[textureUVAnimationLookupIdsOffset], numTextureUVAnimationLookupIds * sizeof(u16));
    }

    // Read Texture Combiner Combos
    {
        if (flags.Use_Texture_Combiner_Combos)
        {
            u32 numTextureCombinerCombos = file.textureCombinerCombos.size;
            u32 textureCombinerCombosOffset = file.textureCombinerCombos.offset;

            textureCombinerCombos.resize(numTextureCombinerCombos);
            memcpy(textureCombinerCombos.data(), &file.m2Buffer->GetDataPointer()[textureCombinerCombosOffset], numTextureCombinerCombos * sizeof(u16));
        }
    }

    // Read Model Data
    {
        // Read Vertex Lookup Ids
        {
            u32 numVertexLookupIds = skin.vertices.size;
            u32 vertexLookupIdsOffset = skin.vertices.offset;

            modelData.vertexLookupIds.resize(numVertexLookupIds);
            memcpy(modelData.vertexLookupIds.data(), &file.skinBuffer->GetDataPointer()[vertexLookupIdsOffset], numVertexLookupIds * sizeof(u16));
        }

        // Read Indices
        {
            u32 numIndicies = skin.indices.size;
            u32 indicesOffset = skin.indices.offset;

            modelData.indices.resize(numIndicies);
            memcpy(modelData.indices.data(), &file.skinBuffer->GetDataPointer()[indicesOffset], numIndicies * sizeof(u16));
        }

        // Read Render Batches
        {
            u32 numRenderBatches = skin.subMeshes.size;
            u32 numTextureUnits = skin.batches.size;

            for (u32 i = 0; i < numRenderBatches; i++)
            {
                ComplexRenderBatch& renderBatch = modelData.renderBatches.emplace_back();
                M2SkinSelection* m2SkinSelection = skin.subMeshes.GetElement(file.skinBuffer, i);

                renderBatch.groupId = m2SkinSelection->skinSectionId;
                renderBatch.vertexStart = m2SkinSelection->vertexStart;
                renderBatch.vertexCount = m2SkinSelection->vertexCount;
                renderBatch.indexStart = m2SkinSelection->indexStart + (m2SkinSelection->level * std::numeric_limits<u16>().max()); // "Level" is an incremental value that goes up once when indexStart goes above the numeric limit of u16 (65536)
                renderBatch.indexCount = m2SkinSelection->indexCount;

                // Read Texture Units
                {
                    renderBatch.textureUnits.reserve(8); // Wowdev.wiki states it is capped at 7 (0 based)

                    for (u32 j = 0; j < numTextureUnits; j++)
                    {
                        M2Batch* m2Batch = skin.batches.GetElement(file.skinBuffer, j);

                        if (m2Batch->skinSectionIndex != i)
                            continue;

                        ComplexTextureUnit& textureUnit = renderBatch.textureUnits.emplace_back();
                        textureUnit.flags = *reinterpret_cast<ComplexTextureUnitFlag*>(&m2Batch->flags);
                        textureUnit.shaderId = m2Batch->shaderId;
                        textureUnit.materialIndex = m2Batch->materialIndex;
                        textureUnit.materialLayer = m2Batch->materialLayer;

                        textureUnit.textureCount = m2Batch->textureCount;

                        for (u16 t = 0; t < textureUnit.textureCount; t++)
                        {
                            textureUnit.textureIndices[t] = textureIndexLookupTable[t + m2Batch->textureLookupId];
                            textureUnit.textureUVAnimationIndices[t] = textureUVAnimationLookupTable[t + m2Batch->textureUVAnimationLookupId];
                        }

                        textureUnit.textureTransparencyLookupId = m2Batch->textureTransparencyLookupId;

                        // Check priorityPlane and take the biggest one for the batch
                        u8 renderPriority = static_cast<u8>(static_cast<i16>(m2Batch->priorityPlane) + 127);
                        if (renderPriority > renderBatch.renderPriority)
                        {
                            renderBatch.renderPriority = renderPriority;
                        }
                    }
                }
            }
        }
    }

    FixData();
    CalculateShaderID();
}

void ComplexModel::FixData()
{
    // Adjust Indicies to refer to the global vertex list
    for (u32 i = 0; i < modelData.indices.size(); i++)
    {
        u16& localVertexIndex = modelData.indices[i];
        localVertexIndex = modelData.vertexLookupIds[localVertexIndex];
    }

    // Clone and invert two sided renderbatches so we don't need to have double pipelines
    u32 numRenderBatches = static_cast<u32>(modelData.renderBatches.size());
    for (u32 i = 0; i < numRenderBatches; i++)
    {
        const ComplexRenderBatch renderBatch = modelData.renderBatches[i];

        u32 numTextureUnits = static_cast<u32>(renderBatch.textureUnits.size());
        if (numTextureUnits > 0)
        {
            for (u32 j = 0; j < numTextureUnits; j++)
            {
                const ComplexTextureUnit& textureUnit = renderBatch.textureUnits[j];
                ComplexMaterial& material = materials[textureUnit.materialIndex];

                if (material.flags.disableBackfaceCulling)
                {
                    ComplexRenderBatch& clonedRenderBatch = modelData.renderBatches.emplace_back();
                    clonedRenderBatch.groupId = renderBatch.groupId;
                    clonedRenderBatch.indexCount = renderBatch.indexCount;
                    clonedRenderBatch.indexStart = renderBatch.indexStart;
                    clonedRenderBatch.textureUnits = renderBatch.textureUnits;
                    clonedRenderBatch.vertexCount = renderBatch.vertexCount;
                    clonedRenderBatch.vertexStart = renderBatch.vertexStart;

                    u32 numIndicesBeforeAdd = static_cast<u32>(modelData.indices.size());
                    u32 indexCount = clonedRenderBatch.indexCount;
                    u32 indexStart = clonedRenderBatch.indexStart;

                    modelData.indices.resize(numIndicesBeforeAdd + indexCount);
                    for (u32 k = 0; k < indexCount; k++)
                    {
                        u32 dst = numIndicesBeforeAdd + k;
                        u32 src = indexStart + (indexCount - k) - 1; // Read the original indices backwards, this is our src

                        modelData.indices[dst] = modelData.indices[src];
                    }

                    clonedRenderBatch.indexStart = numIndicesBeforeAdd;
                    break;
                }
            }
        }
    }
}

void ComplexModel::CalculateShaderID()
{
    static u32 opaqueRenderBatches = 0;
    static u32 transparentRenderBatches = 0;

    for (ComplexRenderBatch& renderBatch : modelData.renderBatches)
    {
        u16 layeredTextureUnits = 0;

        for (ComplexTextureUnit& textureUnit : renderBatch.textureUnits)
        {
            ResolveShaderID1(textureUnit);

            layeredTextureUnits += 1 * textureUnit.materialLayer != 0;
        }

        if (layeredTextureUnits > 0)
            ResolveShaderID2(renderBatch);

        ConvertShaderIDs(renderBatch);
    }
}

void ComplexModel::ResolveShaderID1(ComplexTextureUnit& textureUnit)
{
    // According to Wowdev.wiki textureUnits with shaderID 0x8000 should not be rendered
    if ((textureUnit.shaderId & 0x8000) != 0)
        return;

    ComplexMaterial& material = materials[textureUnit.materialIndex];

    u16 shaderID = 0;

    if (!flags.Use_Texture_Combiner_Combos)
    {
        u16 textureUnitValue = textureUnitLookupTable[textureUnit.textureUnitLookupId];

        bool envMapped = textureUnitValue == std::numeric_limits<u16>().max();
        bool isTransparent = material.blendingMode != 0;

        if (isTransparent)
        {
            shaderID = 0x01;

            if (envMapped)
                shaderID |= 0x08;

            shaderID *= 0x10;
        }

        if (textureUnitValue == 1)
        {
            shaderID |= 0x4000;
        }
    }
    else
    {
        // Name is guessed based on usage below
        u16 blendOverrideModifier[2] = { 0 };

        for (u16 i = 0; i < textureUnit.textureCount; i++)
        {
            u16 blendOverride = textureCombinerCombos[i + textureUnit.shaderId];

            if (i == 0 && material.blendingMode == 0)
                blendOverride = 0;

            u16 textureUnitValue = textureUnitLookupTable[i + textureUnit.textureUnitLookupId];
            bool isEnvMapped = textureUnitValue == std::numeric_limits<u16>().max();
            bool isTransparent = textureUnitValue == 1;

            blendOverrideModifier[i] = blendOverride | (isEnvMapped * 8);

            if (isTransparent && i + 1 == textureUnit.textureCount)
                shaderID |= 0x4000;

        }

        shaderID |= (blendOverrideModifier[0] * 16) | blendOverrideModifier[1];
    }

    textureUnit.shaderId = shaderID;
}

void ComplexModel::ResolveShaderID2(ComplexRenderBatch& renderBatch)
{
    u16 unkFlag1 = 0;
    u16 unkFlag2 = 0;
    bool overrideTransparentLayer = 0;

    ComplexTextureUnit& firstTextureUnit = renderBatch.textureUnits[0];
    ComplexMaterial& firstMaterial = materials[firstTextureUnit.materialIndex];
    const u16 firstLayerTransparencyIndex = textureTransparencyLookupTable[firstTextureUnit.textureTransparencyLookupId];

    for (ComplexTextureUnit& textureUnit : renderBatch.textureUnits)
    {
        u16 textureUnitValue = textureUnitLookupTable[textureUnit.textureUnitLookupId];
        u16 nextTextureUnitValue = textureUnitLookupTable.size() > textureUnit.textureUnitLookupId + 1 ? textureUnitLookupTable[textureUnit.textureUnitLookupId + 1] : 0;

        ComplexMaterial& material = materials[textureUnit.materialIndex];
        const u16 maskedShaderId = textureUnit.shaderId & 0x07;

        if (textureUnit.materialLayer == 0 && textureUnit.textureCount > 0 && material.blendingMode == 0)
        {
            textureUnit.shaderId &= 0xFF8F;
        }

        const u16 layerTransparencyIndex = textureTransparencyLookupTable[firstTextureUnit.textureTransparencyLookupId];

        if (unkFlag1 == 1)
        {
            u16 firstTextureId = firstTextureUnit.textureIndices[0];
            u16 currentTextureId = textureUnit.textureIndices[0];

            if (textureUnit.textureCount == 1 && 
                firstTextureId == currentTextureId && 
                (material.blendingMode == 1 || material.blendingMode == 2) &&
                !firstMaterial.flags.unLit && !material.flags.unLit)
            {
                if (firstLayerTransparencyIndex == layerTransparencyIndex)
                {
                    textureUnit.shaderId = 0x8000;
                    firstTextureUnit.shaderId = 0x8001;
                    
                    // Set unkFlag1 to 2 (This skips boths if statements for unkFlag1)
                    unkFlag1 = 2;
                    continue;
                }

                unkFlag1 = 1;
            }
        }
        else if (unkFlag1 == 0)
        {
            if (textureUnit.textureCount == 2 &&
                material.blendingMode == 0 &&
                (maskedShaderId == 4 || maskedShaderId == 6))
            {
                bool nextIsEnvMapped = nextTextureUnitValue == std::numeric_limits<u16>().max();
                bool currentIsOpaque = textureUnitValue == 0;

                if (currentIsOpaque && nextIsEnvMapped)
                    unkFlag1 = 1;
            }
        }

        if (unkFlag2 == 1)
        {
            bool isEnvMapped = textureUnitValue == std::numeric_limits<u16>().max();
            if (textureUnit.textureCount == 1 && isEnvMapped &&
                (material.blendingMode == 4 || material.blendingMode == 6))
            {
                // Check if we need to merge the two texture units
                if (firstLayerTransparencyIndex == layerTransparencyIndex)
                {
                    unkFlag2 = 2;
                    overrideTransparentLayer = true;

                    textureUnit.shaderId = 0x8000;

                    if (material.blendingMode == 4)
                    {
                        firstTextureUnit.shaderId = 0x8002;
                    }
                    else
                    {
                        firstTextureUnit.shaderId = 0xE;
                    }

                    // Merge the two layers
                    firstTextureUnit.textureCount = 2;
                    firstTextureUnit.textureIndices[1] = textureUnit.textureIndices[0];
                    firstTextureUnit.textureUVAnimationIndices[1] = textureUnit.textureUVAnimationIndices[0];

                    continue;
                }
            }
            else
            {
                unkFlag2 = 0;
            }
        }
        else
        {
            if (unkFlag2 == 2)
            {
                u16 firstTextureId = firstTextureUnit.textureIndices[0];
                u16 currentTextureId = textureUnit.textureIndices[0];

                if (textureUnit.textureCount == 1 &&
                    firstTextureId == currentTextureId &&
                    (material.blendingMode == 1 || material.blendingMode == 2) &&
                    !firstMaterial.flags.unLit && !material.flags.unLit)
                {
                    if (firstLayerTransparencyIndex == layerTransparencyIndex)
                    {
                        // Set unkFlag2 to 3 (This skips boths if statements for unkFlag2)
                        unkFlag2 = 3;

                        textureUnit.shaderId = 0x8000;
                        firstTextureUnit.shaderId = firstTextureUnit.shaderId == 0x8002 ? 0x8003 : 0x8001;
                        continue;
                    }
                }
                else
                {
                    unkFlag2 = 0;
                }
            }
            else if (unkFlag2 != 0)
                continue;
        }

        if (textureUnit.textureCount == 1 && material.blendingMode == 0 && textureUnitValue == 0)
            unkFlag2 = 1;
    }

    if (overrideTransparentLayer)
    {
        // Look into what we need to do if overrideTransparentLayer is true
    }
}

void ComplexModel::ConvertShaderIDs(ComplexRenderBatch& renderBatch)
{
    /*
        Vertex Shader Ids:
        - 0 (Diffuse_T1)
        - 1 (Diffuse_T2)
        - 2 (Diffuse_Env)
        - 3 (Diffuse_T1_Env)
    
        Pixel Shader Ids:
        - 0 ()
        - 0 ()
        - 0 ()
        - 0 ()
    */

    for (ComplexTextureUnit& textureUnit : renderBatch.textureUnits)
    {
        // If the shaderId is 0x8000 we don't need to map anything
        if (textureUnit.shaderId == 0x8000)
            continue;

        u16 shaderId = textureUnit.shaderId & 0x7FFF;
        u16 textureCount = textureUnit.textureCount;
        
        if (textureUnit.shaderId & 0x8000)
        {
            if (shaderId == 1)
            {
                textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T1_Env;
                textureUnit.pixelShaderId = PixelShaderID::Opaque_Mod2xNA_Alpha;
            }
            else if (shaderId == 2)
            {
                textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T1_Env;
                textureUnit.pixelShaderId = PixelShaderID::Opaque_AddAlpha;
            }
            else if (shaderId == 3)
            {
                textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T1_Env;
                textureUnit.pixelShaderId = PixelShaderID::Opaque_AddAlpha_Alpha;
            }
            else
            {
                // If the shaderId is (0x8000 or > 0x8003) we don't need to map anything
                DebugHandler::PrintFatal("Model (%s) with TextureUnit(%u) where ShaderID(%u) could not be converted (1)", name, textureUnit.materialLayer, shaderId);
                textureUnit.shaderId = std::numeric_limits<u16>().max();
            }
        }
        else
        {
            if (textureCount == 0)
            {
                DebugHandler::PrintFatal("Model (%s) with TextureUnit(%u) where TextureCount is 0", name, textureUnit.materialLayer);
                continue;
            }

            uint16_t t1PixelMode = (shaderId >> 4) & 0x7;
            uint16_t t1EnvMapped = (shaderId >> 4) & 0x8;
            u16 textureUnitValue = textureUnitLookupTable[textureUnit.textureUnitLookupId];

            if (textureCount == 1)
            {
                // Resolve Vertex Shader Id
                {
                    if (t1EnvMapped)
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_Env;
                    }
                    else if (textureUnitValue == 0)
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T1;
                    }
                    else
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T2;
                    }
                }

                // Resolve Pixel Shader Id
                {
                    if (t1PixelMode == 0)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Opaque;
                    }
                    /*else if (t1PixelMode == 1)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Mod;
                    }*/
                    else if (t1PixelMode == 2)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Decal;
                    }
                    else if (t1PixelMode == 3)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Add;
                    }
                    else if (t1PixelMode == 4)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Mod2x;
                    }
                    else if (t1PixelMode == 5)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Fade;
                    }
                    else
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Mod;
                    }
                }
            }
            else
            {
                uint16_t t2PixelMode = shaderId & 0x7;
                uint16_t t2EnvMapped = shaderId & 0x8;

                // Resolve Vertex Shader Id
                {
                    if (t1EnvMapped && !t2EnvMapped)
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_Env_T2;
                    }
                    else if (!t1EnvMapped && t2EnvMapped)
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T1_Env;
                    }
                    else if (t1EnvMapped && t2EnvMapped)
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_Env_Env;
                    }
                    else
                    {
                        textureUnit.vertexShaderId = VertexShaderID::DIFFUSE_T1_T2;
                    }
                }

                // Resolve Pixel Shader Id
                {
                    if (t1PixelMode == 0)
                    {
                        if (t2PixelMode == 0)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_Opaque;
                        }
                        /*else if (t2PixelMode == 1)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_Mod;
                        }*/
                        else if (t2PixelMode == 3)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_Add;
                        }
                        else if (t2PixelMode == 4)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_Mod2x;
                        }
                        else if (t2PixelMode == 6)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_Mod2xNA;
                        }
                        else if (t2PixelMode == 7)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_AddNA;
                        }
                        else
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Opaque_Mod;
                        }
                    }
                    else if (t1PixelMode == 1)
                    {
                        if (t2PixelMode == 0)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Mod_Opaque;
                        }
                        else if (t2PixelMode == 3)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Mod_Add;
                        }
                        else if (t2PixelMode == 4)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Mod_Mod2x;
                        }
                        else if (t2PixelMode == 6)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Mod_Mod2xNA;
                        }
                        else if (t2PixelMode == 7)
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Mod_AddNA;
                        }
                        else
                        {
                            textureUnit.pixelShaderId = PixelShaderID::Mod_Mod;
                        }
                    }
                    else if (t1PixelMode == 3 && t2PixelMode == 1)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Add_Mod;
                    }
                    else if (t1PixelMode == 4 && t2PixelMode == 1)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Mod2x_Mod;
                    }
                    else if (t1PixelMode == 4 && t2PixelMode == 4)
                    {
                        textureUnit.pixelShaderId = PixelShaderID::Mod2x_Mod2x;
                    }
                    else
                    {
                        DebugHandler::PrintFatal("Model (%s) with TextureUnit(%u) where ShaderID(t1PixelMode: %u, t2PixelMode: %u) could not be converted (2)", name, textureUnit.materialLayer, t1PixelMode, t2PixelMode);
                        textureUnit.shaderId = std::numeric_limits<u16>().max();
                    }
                }
            }
        }
    }
}

void ComplexModel::SaveToDisk(const fs::path& filePath)
{
    ZoneScopedN("ComplexModel::SaveToFile");

    // Create a file
    std::ofstream output(filePath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create ComplexModel file. Check admin permissions\n");
        return;
    }

    // Write Complex Model Header
    {
        output.write(reinterpret_cast<char const*>(&header), sizeof(header));
        output.write(reinterpret_cast<char const*>(&flags), sizeof(flags));
    }

    // Write Sequences
    {
        u32 numElements = static_cast<u32>(sequences.size());
        output.write(reinterpret_cast<char const*>(&numElements), sizeof(numElements));

        if (numElements > 0)
        {
            output.write(reinterpret_cast<char const*>(&sequences[0]), numElements * sizeof(ComplexAnimationSequence));
        }
    }

    // Write Bones
    {
        u32 numElements = static_cast<u32>(bones.size());
        output.write(reinterpret_cast<char const*>(&numElements), sizeof(numElements));

        if (numElements > 0)
        {
            for (const ComplexBone& bone : bones)
            {
                output.write(reinterpret_cast<char const*>(&bone.primaryBoneIndex), sizeof(bone.primaryBoneIndex));
                output.write(reinterpret_cast<char const*>(&bone.flags), sizeof(bone.flags));

                output.write(reinterpret_cast<char const*>(&bone.parentBoneId), sizeof(bone.parentBoneId));
                output.write(reinterpret_cast<char const*>(&bone.submeshId), sizeof(bone.submeshId));

                bone.translation.Serialize(output);
                bone.rotation.Serialize(output);
                bone.scale.Serialize(output);

                output.write(reinterpret_cast<char const*>(&bone.pivot), sizeof(bone.pivot));
            }
        }
    }
    
    // Write Vertices
    {
        u32 numVertices = static_cast<u32>(vertices.size());
        output.write(reinterpret_cast<char const*>(&numVertices), sizeof(numVertices));

        if (numVertices > 0)
        {
            output.write(reinterpret_cast<char const*>(&vertices[0]), numVertices * sizeof(ComplexVertex));
        }
    }

    // Write Textures
    {
        u32 numTextures = static_cast<u32>(textures.size());
        output.write(reinterpret_cast<char const*>(&numTextures), sizeof(numTextures));

        if (numTextures > 0)
        {
            for each (ComplexTexture texture in textures)
            {
                output.write(reinterpret_cast<char const*>(&texture.type), sizeof(texture.type));
                output.write(reinterpret_cast<char const*>(&texture.flags), sizeof(texture.flags));

                u32 textureNameHash = std::numeric_limits<u32>().max();

                if (texture.type == ComplexTextureType::NONE)
                {
                    fs::path texturePath = texture.fileName;
                    texturePath.replace_extension("dds").make_preferred();

                    std::string texturePathStr = texturePath.string();
                    std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

                    textureNameHash = StringUtils::fnv1a_32(texturePathStr.c_str(), texturePathStr.length());
                }

                output.write(reinterpret_cast<char const*>(&textureNameHash), sizeof(textureNameHash));
            }
        }
    }

    // Write Materials
    {
        u32 numMaterials = static_cast<u32>(materials.size());
        output.write(reinterpret_cast<char const*>(&numMaterials), sizeof(numMaterials));

        if (numMaterials > 0)
        {
            output.write(reinterpret_cast<char const*>(&materials[0]), numMaterials * sizeof(ComplexMaterial));
        }
    }

    // Write Texture Index Lookup Table
    {
        u32 numTextureIndexLookupIds = static_cast<u32>(textureIndexLookupTable.size());
        output.write(reinterpret_cast<char const*>(&numTextureIndexLookupIds), sizeof(numTextureIndexLookupIds));

        if (numTextureIndexLookupIds > 0)
        {
            output.write(reinterpret_cast<char const*>(&textureIndexLookupTable[0]), numTextureIndexLookupIds * sizeof(u16));
        }
    }

    // Write Texture Unit Lookup Table
    {
        u32 numTextureUnitLookupIds = static_cast<u32>(textureUnitLookupTable.size());
        output.write(reinterpret_cast<char const*>(&numTextureUnitLookupIds), sizeof(numTextureUnitLookupIds));

        if (numTextureUnitLookupIds > 0)
        {
            output.write(reinterpret_cast<char const*>(&textureUnitLookupTable[0]), numTextureUnitLookupIds * sizeof(u16));
        }
    }

    // Write Texture Transparency Lookup Table
    {
        u32 numTextureTransparencyLookupIds = static_cast<u32>(textureTransparencyLookupTable.size());
        output.write(reinterpret_cast<char const*>(&numTextureTransparencyLookupIds), sizeof(numTextureTransparencyLookupIds));

        if (numTextureTransparencyLookupIds > 0)
        {
            output.write(reinterpret_cast<char const*>(&textureTransparencyLookupTable[0]), numTextureTransparencyLookupIds * sizeof(u16));
        }
    }

    // Write Texture Combiner Combos
    {
        u32 numTextureCombinerCombos = static_cast<u32>(textureCombinerCombos.size());
        output.write(reinterpret_cast<char const*>(&numTextureCombinerCombos), sizeof(numTextureCombinerCombos));

        if (numTextureCombinerCombos > 0)
        {
            output.write(reinterpret_cast<char const*>(&textureCombinerCombos[0]), numTextureCombinerCombos * sizeof(u16));
        }
    }

    // Write Model Data
    {
        // Write Complex Model Data Header
        output.write(reinterpret_cast<char const*>(&modelData.header), sizeof(modelData.header));
        
        // Write Vertex Lookup Ids
        {
            u32 numVertexLookupIds = static_cast<u32>(modelData.vertexLookupIds.size());
            output.write(reinterpret_cast<char const*>(&numVertexLookupIds), sizeof(numVertexLookupIds));

            if (numVertexLookupIds > 0)
            {
                output.write(reinterpret_cast<char const*>(&modelData.vertexLookupIds[0]), numVertexLookupIds * sizeof(u16));
            }
        }

        // Write Indices
        {
            u32 numIndices = static_cast<u32>(modelData.indices.size());
            output.write(reinterpret_cast<char const*>(&numIndices), sizeof(numIndices));

            if (numIndices > 0)
            {
                output.write(reinterpret_cast<char const*>(&modelData.indices[0]), numIndices * sizeof(u16));
            }
        }

        // Write Render Batches
        {
            u32 numRenderBatches = static_cast<u32>(modelData.renderBatches.size());
            output.write(reinterpret_cast<char const*>(&numRenderBatches), sizeof(numRenderBatches));

            for (u32 i = 0; i < numRenderBatches; i++)
            {
                ComplexRenderBatch& renderBatch = modelData.renderBatches[i];

                output.write(reinterpret_cast<char const*>(&renderBatch.groupId), sizeof(renderBatch.groupId));
                output.write(reinterpret_cast<char const*>(&renderBatch.vertexStart), sizeof(renderBatch.vertexStart));
                output.write(reinterpret_cast<char const*>(&renderBatch.vertexCount), sizeof(renderBatch.vertexCount));
                output.write(reinterpret_cast<char const*>(&renderBatch.indexStart), sizeof(renderBatch.indexStart));
                output.write(reinterpret_cast<char const*>(&renderBatch.indexCount), sizeof(renderBatch.indexCount));

                u32 numTextureUnits = static_cast<u32>(renderBatch.textureUnits.size());
                output.write(reinterpret_cast<char const*>(&numTextureUnits), sizeof(numTextureUnits));

                for (u32 j = 0; j < numTextureUnits; j++)
                {
                    ComplexTextureUnit& textureUnit = renderBatch.textureUnits[j];

                    output.write(reinterpret_cast<char const*>(&textureUnit.flags), sizeof(textureUnit.flags));
                    output.write(reinterpret_cast<char const*>(&textureUnit.shaderId), sizeof(textureUnit.shaderId));
                    output.write(reinterpret_cast<char const*>(&textureUnit.materialIndex), sizeof(textureUnit.materialIndex));
                    output.write(reinterpret_cast<char const*>(&textureUnit.materialLayer), sizeof(textureUnit.materialLayer));
                    output.write(reinterpret_cast<char const*>(&textureUnit.textureCount), sizeof(textureUnit.textureCount));
                    output.write(reinterpret_cast<char const*>(&textureUnit.textureIndices), textureUnit.textureCount * sizeof(u16));
                    output.write(reinterpret_cast<char const*>(&textureUnit.textureUVAnimationIndices), textureUnit.textureCount * sizeof(u16));
                    output.write(reinterpret_cast<char const*>(&textureUnit.textureUnitLookupId), sizeof(textureUnit.textureUnitLookupId));
                }
            }
        }
    }

    // Write CullingData
    {
        output.write(reinterpret_cast<char const*>(&cullingData), sizeof(CullingData));
    }

    output.close();
}