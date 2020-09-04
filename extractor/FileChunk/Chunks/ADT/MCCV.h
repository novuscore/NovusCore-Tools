#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Vertex shading information Chunk
struct MCCV
{
    struct MCCVEntry
    {
        u8 blue = 127;
        u8 green = 127;
        u8 red = 127;
        u8 alpha = 127;
    } entries[9*9+8*8];
};
#pragma pack(pop)