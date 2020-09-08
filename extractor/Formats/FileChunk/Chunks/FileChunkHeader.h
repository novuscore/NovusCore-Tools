#pragma once
#include <NovusTypes.h>

#pragma pack(push, 1)
struct FileChunkHeader
{
    u32 token;
    u32 size;
};

struct IntColor
{
    union
    {
        u32 bgra = 0;
        struct {
            u8 b;
            u8 g;
            u8 r;
            u8 a;
        };
    };
};
#pragma pack(pop)