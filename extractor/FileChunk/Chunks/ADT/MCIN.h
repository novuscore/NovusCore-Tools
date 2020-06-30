#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Map Cell Mapping Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MCIN
{
    struct MCNKInfo
    {
        u32 offset = 0; // Absolute
        u32 size = 0;
        u32 flags = 0; // Always 0. Only set in the client (FLAG_LOADED = 1)

        u8 padding[4] = { 0 }; // This is used inside the client (asyncId)
    } mcnkInfo[16*16];

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
