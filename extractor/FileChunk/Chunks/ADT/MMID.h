#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// M2 Chunk (Describes an offset into MMDX. These offsets are referenced in MDDF)
struct WDT;
struct ADT;
struct ChunkHeader;
struct MMID
{
    u32 size;
    u8* offsets; // Array of u32s

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
