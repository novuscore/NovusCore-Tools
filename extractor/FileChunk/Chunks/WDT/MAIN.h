#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

constexpr u32 NUM_SM_AREA_INFO = 64 * 64;

#pragma pack(push, 1)
// Cell Information Chunk
struct WDT;
struct ChunkHeader;
struct MAIN
{
    struct SMAreaInfo
    {
        u32 hasADT = 0;
        u32 loaded = 0; // Unused
    };
    SMAreaInfo MapAreaInfo[NUM_SM_AREA_INFO];

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& wdt);
};
#pragma pack(pop)
