#pragma once
#include <NovusTypes.h>

#pragma pack(push, 1)
struct ChunkHeader
{
    u32 token;
    u32 size;
};
#pragma pack(pop)