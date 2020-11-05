#pragma once
#include "NovusTypes.h"
#include "NovusTypeHeader.h"

#include "M2.h"

#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct ComplexVertex
{
    hvec3 position = hvec3(f16(0), f16(0), f16(0));
    u8 octNormal[2] = { 0 };
    hvec2 uvCords[2] = { hvec2(f16(0), f16(0)), hvec2(f16(0), f16(0)) };
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
    uvec2 repetitionRange; // Unless the value is (0,0) pick a random number of repetitions to do based on (min, max)
    u16 blendTimeStart = 0;
    u16 blendTimeEnd = 0;

    vec3 boundingBoxExtentmin;
    vec3 boundingBoxExtentmax;
    f32 radius;

    i16 nextVariationId = 0; // Specifies the variation id for the next variation for this animation or (-1) for none.
    u16 nextAliasId = 0; // Specifies the id for the actual animation.
};

enum class AnimationTrackInterpolationType
{
    NONE,
    LINEAR,
    CUBIC_BEZIER_SPLINE, // Only used for M2SplineKey tracks (WowDev.Wiki states Bezier/Hermit might be in the wrong order)
    CUBIC_HERMIT_SPLINE // Only used for M2SplineKey tracks (WowDev.Wiki states Bezier/Hermit might be in the wrong order)
};

enum class ComplexAnimationTrackType
{
    VERTEX_POSITION,
    VERTEX_ROTATION,
    VERTEX_SCALE,
    UV_POSITION,
    UV_SCALE
};

struct ComplexAnimationTrack
{
    ComplexAnimationTrack() { }
    ComplexAnimationTrack(ComplexAnimationTrackType trackType) : type(trackType) { }

    ComplexAnimationTrackType type;
    AnimationTrackInterpolationType interpolationType;
    std::vector<u16> sequencesInUse;

    // Data is loaded from the sequence
};

struct ComplexTextureTransform
{
    // Check https://wowdev.wiki/M2#Texture_Transforms

    ComplexAnimationTrack position = ComplexAnimationTrack(ComplexAnimationTrackType::VERTEX_POSITION);
    ComplexAnimationTrack rotation = ComplexAnimationTrack(ComplexAnimationTrackType::VERTEX_ROTATION); 
    ComplexAnimationTrack scaling = ComplexAnimationTrack(ComplexAnimationTrackType::VERTEX_SCALE);
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
    hvec3 minBoundingBox = hvec3(static_cast<f16>(100000.0f));
    hvec3 maxBoundingBox = hvec3(static_cast<f16>(-100000.0f));
    f32 boundingSphereRadius = 0.0f;
}; // 16 bytes

struct M2File;
struct ComplexModel
{
public:
    NovusTypeHeader header = NovusTypeHeader(10, 2);

    char* name;
    ComplexModelFlag flags;
    ComplexModelData modelData;

    std::vector<u32> animationFileNameHashes;

    std::vector<M2Vertex> m2Vertices;

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
    void FixData();
    void CalculateShaderID();
    void ResolveShaderID1(ComplexTextureUnit& textureUnit);
    void ResolveShaderID2(ComplexRenderBatch& renderBatch);
    void ConvertShaderIDs(ComplexRenderBatch& renderBatch);
};
#pragma pack(pop)