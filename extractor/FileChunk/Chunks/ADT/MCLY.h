#pragma once
#include <NovusTypes.h>
#include "../ChunkHeader.h"

enum class MCLYFlags
{
    AnimationRotation_X = 1 << 0,
    AnimationRotation_Y = 1 << 1,
    AnimationRotation_Z = 1 << 2,
    AnimationSpeed_X = 1 << 3,
    AnimationSpeed_Y = 1 << 4,
    AnimationSpeed_Z = 1 << 5,
    AnimationEnabled = 1 << 6,
    Overbright = 1 << 7,
    UseAlphaMap = 1 << 8,
    CompressedAlphaMap = 1 << 9,
    UseCubeMapReflection = 1 << 10,
    Unk_0x800 = 1 << 11,
    Unk_0x1000 = 1 << 12,

    // Remaining 19 bits are unused
};

#pragma pack(push, 1)
// M2 Chunk (Describes which M2s are used in this map chunk. These offsets are referenced in MMID)
struct MCLY
{
    u32 textureId = 0;
    u32 flags = 0;
    u32 offsetInMCAL = 0;
    u32 effectId = 0;
};
#pragma pack(pop)
