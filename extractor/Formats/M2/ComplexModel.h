#pragma once
#include "NovusTypes.h"
#include "NovusTypeHeader.h"

#include "M2.h"

#include <Utils/ByteBuffer.h>
#include <Utils/DynamicBytebuffer.h>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct ComplexVertex
{
    hvec3 position = hvec3(f16(0), f16(0), f16(0));
    u8 octNormal[2] = { 0 };
    hvec2 uvCords[2] = { hvec2(f16(0), f16(0)), hvec2(f16(0), f16(0)) };

    u8 boneIndices[4] = { 0, 0, 0, 0 };
    u8 boneWeights[4] = { 0, 0, 0, 0 };
};

enum class ComplexTextureType : u32
{
    NONE = 0,
    COMPONENT_SKIN,
    COMPONENT_OBJECT_SKIN,
    COMPONENT_WEAPON_BLADE,
    COMPONENT_WEAPON_HANDLE,
    COMPONENT_ENVIRONMENT, // OBSOLETE
    COMPONENT_CHAR_HAIR,
    COMPONENT_CHAR_FACIAL_HAIR, // OBSOLETE
    COMPONENT_SKIN_EXTRA,
    COMPONENT_UI_SKIN,
    COMPONENT_TAUREN_MANE, // OBSOLETE
    COMPONENT_MONSTER_SKIN_1,
    COMPONENT_MONSTER_SKIN_2,
    COMPONENT_MONSTER_SKIN_3,
    COMPONENT_ITEM_ICON
};

struct ComplexTextureFlag
{
    u32 wrapX : 1;
    u32 wrapY : 1;
};

struct ComplexTexture
{
    ComplexTextureType type = ComplexTextureType::NONE; // Check https://wowdev.wiki/M2#Textures
    ComplexTextureFlag flags;

    char* fileName; // This is only valid if type == 0, however for non 0 types, this still points to a 0 terminated string
};

struct ComplexMaterialFlag
{
    u16 unLit : 1;
    u16 unFogged : 1;
    u16 disableBackfaceCulling : 1;
    u16 depthTest : 1;
    u16 depthWrite : 1;
    u16 : 5;
};

struct ComplexMaterial
{
    ComplexMaterialFlag flags;
    u16 blendingMode; // Check https://wowdev.wiki/M2/Rendering#M2BLEND
};

enum class AnimationTrackInterpolationType : u8
{
    NONE,
    LINEAR,
    CUBIC_BEZIER_SPLINE, // Only used for M2SplineKey tracks (WowDev.Wiki states Bezier/Hermit might be in the wrong order)
    CUBIC_HERMIT_SPLINE // Only used for M2SplineKey tracks (WowDev.Wiki states Bezier/Hermit might be in the wrong order)
};

template <typename T>
struct ComplexAnimationTrack
{
    u32 sequenceId;
    std::vector<u32> timestamps;
    std::vector<T> values;
};

template <typename T>
struct ComplexAnimationData
{
    ComplexAnimationData() { }

    AnimationTrackInterpolationType interpolationType = AnimationTrackInterpolationType::NONE;
    bool isGlobalSequence = false;

    std::vector<ComplexAnimationTrack<T>> tracks;

    void Serialize(std::ofstream& stream) const
    {
        stream.write(reinterpret_cast<char const*>(&interpolationType), sizeof(interpolationType));
        stream.write(reinterpret_cast<char const*>(&isGlobalSequence), sizeof(isGlobalSequence));

        u32 numTracks = static_cast<u32>(tracks.size());
        {
            stream.write(reinterpret_cast<char const*>(&numTracks), sizeof(numTracks));

            for (u32 i = 0; i < numTracks; i++)
            {
                const ComplexAnimationTrack<T>& track = tracks[i];

                stream.write(reinterpret_cast<char const*>(&track.sequenceId), sizeof(track.sequenceId));

                u32 numTimestamps = static_cast<u32>(track.timestamps.size());
                {
                    stream.write(reinterpret_cast<char const*>(&numTimestamps), sizeof(numTimestamps));
                    stream.write(reinterpret_cast<char const*>(track.timestamps.data()), numTimestamps * sizeof(u32));
                }

                u32 numValues = static_cast<u32>(track.values.size());
                {
                    stream.write(reinterpret_cast<char const*>(&numValues), sizeof(numValues));
                    stream.write(reinterpret_cast<char const*>(track.values.data()), numTimestamps * sizeof(T));
                }
            }
        }
    }
    bool Deserialize(Bytebuffer* buffer)
    {
        if (!buffer->Get(interpolationType))
            return false;

        if (!buffer->Get(isGlobalSequence))
            return false;

        u32 numTracks = 0;
        {
            if (!buffer->GetU32(numTracks))
                return false;

            tracks.resize(numTracks);

            for (u32 i = 0; i < numTracks; i++)
            {
                ComplexAnimationTrack<T>& track = tracks[i];

                if (!buffer->GetU32(track.sequenceId))
                    return false;

                u32 numTimestamps = 0;
                if (!buffer->GetU32(numTimestamps))
                    return false;

                track.timestamps.resize(numTimestamps);
                if (!buffer->GetBytes(reinterpret_cast<u8*>(track.timestamps.data()), numTimestamps * sizeof(u32)))
                    return false;

                u32 numValues = 0;
                if (!buffer->GetU32(numValues))
                    return false;

                track.values.resize(numValues);
                if (!buffer->GetBytes(reinterpret_cast<u8*>(track.values.data()), numValues * sizeof(T)))
                    return false;
            }
        }
    }
};

struct ComplexAnimationSequence
{
    u16 id = 0; // Animation Id (AnimationData.ndbc)
    u16 subId = 0; // Sub Animation Id (An id to indicate the variation index of this animation)

    u32 duration = 0; // Duration of animation in milliseconds.
    f32 moveSpeed = 0.f; // The speed at which the model moves with.

    struct ComplexAnimationSequenceFlag
    {
        u32 isAlwaysPlaying : 1;
        u32 isAlias : 1;
        u32 blendTransition : 1; // (This applies if set on either side of the transition) If set we lerp between the end -> start states, but only if end != start (Compare Bone Values)
    } flags;

    i16 frequency = 0; // Determines how often the animation is played.
    uvec2 repetitionRange = uvec2(0, 0); // Unless the value is (0,0) pick a random number of repetitions to do based on (min, max)
    u16 blendTimeStart = 0;
    u16 blendTimeEnd = 0;

    vec3 extentsMin = vec3(0.0f, 0.0f, 0.0f);
    vec3 extentsMax = vec3(0.0f, 0.0f, 0.0f);
    f32 radius = 0;

    i16 nextVariationId = -1; // Specifies the variation id for the next variation for this animation or (-1) for none.
    u16 nextAliasId = 0; // Specifies the id for the actual animation.
};

struct ComplexBone
{
    i32 primaryBoneIndex = -1;

    struct Flags
    {
        u32 ignoreParentTranslate : 1;
        u32 ignoreParentScale : 1;
        u32 ignoreParentRotation : 1;
        u32 sphericalBillboard : 1;
        u32 cylindricalBillboard_LockX : 1;
        u32 cylindricalBillboard_LockY : 1;
        u32 cylindricalBillboard_LockZ : 1;
        u32 unk_0x80 : 1;
        u32 : 1;
        u32 transformed : 1;
        u32 kinematicBone : 1;
        u32 : 1;
        u32 helmetAnimationScale : 1;
        u32 unk_0x1000 : 1;
    } flags;

    i16 parentBoneId = -1;
    u16 submeshId = 0;

    ComplexAnimationData<vec3> translation;
    ComplexAnimationData<vec4> rotation;
    ComplexAnimationData<vec3> scale;

    vec3 pivot;
};

struct ComplexTextureTransform
{
    // Check https://wowdev.wiki/M2#Texture_Transforms

    ComplexAnimationData<vec3> position;
    ComplexAnimationData<vec4> rotation;
    ComplexAnimationData<vec3> scaling;
};

// Check https://wowdev.wiki/M2/.skin#Texture_units
enum class ComplexTextureUnitFlag : u8
{
    ANIMATED = 0,
    INVERTED_MATERIALS = 1,
    TRANSFORM = 2,
    PROJECTED_TEXTURE = 4,
    STATIC_TEXTURE = 16,
};

struct ComplexTextureUnit
{
    ComplexTextureUnitFlag flags;

    union
    {
        u16 shaderId;
        struct
        {
            u8 vertexShaderId;
            u8 pixelShaderId;
        };
    };

    u16 materialIndex;
    u16 materialLayer;

    u16 textureCount;
    u16 textureIndices[2] = { 0, 0 };
    u16 textureUVAnimationIndices[2] = { 0, 0 };
    u16 textureUnitLookupId;
    u16 textureTransparencyLookupId;

    // TODO: Add the remaining data later
};

struct ComplexRenderBatch
{
    u16 groupId = 0;
    u32 vertexStart = 0;
    u32 vertexCount = 0;
    u32 indexStart = 0;
    u32 indexCount = 0;
    u8 renderPriority = 0;

    // TODO: Add the remaining data later

    std::vector<ComplexTextureUnit> textureUnits;
};

struct ComplexModelData
{
    NovusTypeHeader header = NovusTypeHeader(11, 1);

    std::vector<u16> vertexLookupIds;
    std::vector<u16> indices; // These are relative to the index of vertexLookupIds and needs to be translated
    std::vector<ComplexRenderBatch> renderBatches;

    // TODO: Add the remaining data later
};

struct ComplexModelFlag
{
    u32 Tilt_X : 1;
    u32 Tilt_Y : 1;
    u32 : 1;
    u32 Use_Texture_Combiner_Combos : 1; // (TBC+)
    u32 : 1; // (TBC+)
    u32 Load_Physics_Data : 1; // (MOP+)
    u32 : 1; // (MOP+)
    u32 Unk_0x80 : 1; // (WOD+)
    u32 Camera_Related : 1; // (WOD+)
    u32 New_Particle_Record : 1; // (Legion+)
    u32 Unk_0x400 : 1; // (Legion+)
    u32 Texture_Transforms_Use_Bone_Sequences : 1; // (Legion+)

    // 0x1000 to 0x200000 are unk (Legion+)
};

enum VertexShaderID : u8
{
    DIFFUSE_T1,
    DIFFUSE_T2,
    DIFFUSE_Env,
    DIFFUSE_T1_T2,
    DIFFUSE_T1_Env,
    DIFFUSE_Env_T2,
    DIFFUSE_Env_Env,
};

enum PixelShaderID : u8
{
    Opaque,
    Opaque_Opaque,
    Opaque_Mod,
    Opaque_Mod2x,
    Opaque_Mod2xNA,
    Opaque_Add,
    Opaque_AddNA,
    Opaque_AddAlpha,
    Opaque_AddAlpha_Alpha,
    Opaque_Mod2xNA_Alpha,
    Mod,
    Mod_Opaque,
    Mod_Mod,
    Mod_Mod2x,
    Mod_Mod2xNA,
    Mod_Add,
    Mod_AddNA,
    Mod2x,
    Mod2x_Mod,
    Mod2x_Mod2x,
    Add,
    Add_Mod,
    Fade,
    Decal
};

struct CullingData
{
    hvec3 minBoundingBox = hvec3(static_cast<f16>(65535.0f));
    hvec3 maxBoundingBox = hvec3(static_cast<f16>(-65535.0f));
    f32 boundingSphereRadius = 0.0f;
}; // 16 bytes

struct M2File;
struct ComplexModel
{
public:
    NovusTypeHeader header = NovusTypeHeader(10, 4);

    char* name;
    ComplexModelFlag flags;
    ComplexModelData modelData;

    std::vector<u32> animationFileNameHashes;

    std::vector<ComplexAnimationSequence> sequences;
    std::vector<ComplexBone> bones;

    std::vector<ComplexVertex> vertices;
    std::vector<ComplexTexture> textures;
    std::vector<ComplexMaterial> materials;
    std::vector<ComplexTextureTransform> textureTransforms;

    std::vector<u16> textureIndexLookupTable;
    std::vector<u16> textureUnitLookupTable;
    std::vector<u16> textureTransparencyLookupTable;
    std::vector<u16> textureUVAnimationLookupTable;

    std::vector<u16> textureCombinerCombos;

    CullingData cullingData;

public:
    void ReadFromM2(M2File& file);
    void SaveToDisk(const fs::path& filePath);

private:

    template <typename T>
    void FillAnimationTrackFromM2Track(M2File& file, ComplexAnimationData<T>& animationData, M2Track<T>& m2Track)
    {
        u32 numTracks = m2Track.values.size;
        
        animationData.interpolationType = AnimationTrackInterpolationType::LINEAR;
        animationData.isGlobalSequence = m2Track.globalSequence != -1;
        animationData.tracks.reserve(numTracks);

        for (u32 i = 0; i < numTracks; i++)
        {
            M2Array<u32>* m2Timestamps = m2Track.timestamps.GetElement(file.m2Buffer, i);
            M2Array<T>* m2Values = m2Track.values.GetElement(file.m2Buffer, i);

            if (m2Timestamps->size > 0 && m2Values->size > 0)
            {
                ComplexAnimationTrack<T>& track = animationData.tracks.emplace_back();

                track.sequenceId = i;

                track.timestamps.resize(m2Timestamps->size);
                memcpy(track.timestamps.data(), m2Timestamps->Get(file.m2Buffer), sizeof(u32) * m2Timestamps->size);

                track.values.resize(m2Values->size);
                memcpy(track.values.data(), m2Values->Get(file.m2Buffer), sizeof(T) * m2Values->size);
            }
        }
    }

    void FixData();
    void CalculateShaderID();
    void ResolveShaderID1(ComplexTextureUnit& textureUnit);
    void ResolveShaderID2(ComplexRenderBatch& renderBatch);
    void ConvertShaderIDs(ComplexRenderBatch& renderBatch);
};
#pragma pack(pop)