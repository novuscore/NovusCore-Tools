#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// WMO Chunk (Describes which WMOs are used in this map chunk. These offsets are referenced in MWID)
struct WDT;
struct ADT;
struct ChunkHeader;
struct MWMO
{
    u32 size = 0;
    u8* filenames = nullptr; // Can store multiple names

    static bool ReadWDT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& adt);
    static bool ReadADT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
