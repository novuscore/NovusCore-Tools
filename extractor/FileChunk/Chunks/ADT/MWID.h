#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// WMO Chunk (Describes an offset into MWID. These offsets are referenced in MODF)
struct WDT;
struct ADT;
struct ChunkHeader;
struct MWID
{
    u32 size = 0;
    u8* offset = nullptr; // Array of u32s

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
