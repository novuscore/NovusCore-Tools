#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include "../../ChunkHeader.h"

enum class MOHDFlags
{
    DontAttenuateVerticiesBasedOnDistanceToPortal = 1 << 0,
    UseUnifiedRenderPath = 1 << 1,
    UseLiquidTypeDbcId = 1 << 2,
    Lod = 1 << 3, // Legion+
    DefaultMaxLod = 1 << 4, // Legion+
    Unk_0x32 = 1 << 5 // Unused as of Legion
};

struct WMO_ROOT;
struct MOHD
{
    u32 texturesNum = 0;
    u32 groupsNum = 0;
    u32 portalsNum = 0;
    u32 lightsNum = 0;
    u32 doodadNamesNum = 0;
    u32 doodadDefsNum = 0;
    u32 doodadSetsNum = 0;
    IntColor ambientColor; // This is (ARGB)
    u32 wmoId = 0; // Reference key (WMOAreaTable)
    vec3 boundingBoxMin; // Axis Aligned Box Min
    vec3 boundingBoxMax; // Axis Aligned box Max
    u16 flags = 0;
    u16 lodsNum = 0;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};