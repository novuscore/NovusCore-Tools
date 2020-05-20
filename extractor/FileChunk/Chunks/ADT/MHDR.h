#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MHDRFlags
{
    MFBO = 1 << 0,
    NORTHREND = 1 << 1
};

#pragma pack(push, 1)
// Map Header Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MHDR
{
    u32 flags = 0;

    u32 mcin = 0;
    u32 mtex = 0;
    u32 mmdx = 0;
    u32 mmid = 0;
    u32 mwmo = 0;
    u32 mwid = 0;
    u32 mddf = 0;
    u32 modf = 0;
    u32 mfbo = 0;
    u32 mh2o = 0;
    u32 mtxf = 0;
    u8 mamp_value = 0;
    u8 padding[3] = { 0 };
    u32 unused[3] = { 0 };

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
