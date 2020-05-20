#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Vertex shading information Chunk
struct MCCV
{
    struct MCCVEntry
    {
        u8 blue;
        u8 green;
        u8 red;
        u8 alpha;
    } entries[9*9+8*8];
};
#pragma pack(pop)