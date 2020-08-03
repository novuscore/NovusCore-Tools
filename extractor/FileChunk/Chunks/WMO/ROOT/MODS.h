#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct WMO_ROOT;
struct ChunkHeader;
struct MODS
{
    char name[20] = { 0 };
    u32 startIndex = 0; // Index into MODD
    u32 count = 0;
    char pad[4] = { 0 };

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};