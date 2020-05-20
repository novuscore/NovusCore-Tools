#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// M2 Chunk (Describes which M2s are used in this map chunk. These offsets are referenced in MMID)
struct WDT;
struct ADT;
struct ChunkHeader;
struct MMDX
{
    u32 size = 0;
    u8* filenames = nullptr; // Can store multiple names

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
