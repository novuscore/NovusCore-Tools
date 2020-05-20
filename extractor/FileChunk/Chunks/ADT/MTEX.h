#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Texture List Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MTEX
{
    u32 size = 0;
    u8* filenames = nullptr; // Can store multiple names

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
