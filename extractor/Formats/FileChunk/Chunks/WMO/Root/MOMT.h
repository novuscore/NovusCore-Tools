#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

enum class MOMTFlags
{
    DisableLightingInShader = 1 << 0,
    DisableFogInShader = 1 << 1,
    DisableCulling = 1 << 2,
    EXTLIGHT = 1 << 3,
    SIDN = 1 << 4,
    WINDOW = 1 << 5,
    CLAMP_S = 1 << 6,
    CLAMP_T = 1 << 7,
    Unk_0x100 = 1 << 8
};

struct WMO_ROOT;
struct FileChunkHeader;
struct MOMT
{
    struct MOMTData
    {
        u32 flags = 0;
        u32 shader = 0;
        u32 blendMode = 0;

        u32 textureOffset1 = 0; // Offset into MOTX

        u32 sIDNEmissiveColor = 0; // This is (BGRA)
        u32 sIDNFrameEmissiveColor = 0; // This is (BGRA) (Set at runtime)

        u32 textureOffset2 = 0; // Offset into MOTX
        u32 diffuseColor1 = 0; // // This is (ARGB)

        u32 terrainType = 0; // Reference key (TerrainTypeRec)

        u32 textureOffset3 = 0; // Offset into MOTX
        u32 diffuseColor2 = 0; // This is (ARGB) ??
        u32 texture3Flags = 0;

        u32 unused[4] = { 0 }; // This is padding for runtime data (Usually stores textures)
    };
    
    std::vector<MOMTData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};