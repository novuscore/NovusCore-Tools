#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Version Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MVER
{
    u32 version;

    static bool ReadWDT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& wdt);
    static bool ReadADT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)