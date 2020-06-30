#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Liquid Information Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MH2O
{
    struct LiquidChunk
    {
        u32 instancesOffset = 0;
        u32 layerCount = 0;
        u32 attributesOffset = 0;
    } liquidChunks[16*16];

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};

struct MH2O_LiquidInstance
{
    u32 liquidType = 0; // Foreign Key (Referencing LiquidType.dbc)

    u16 liquidVertexFormat = 0; // Classic, TBC and WOTLK Only (Cata+ Foreign Key)
    f32 minHeightLevel = 0;
    f32 maxHeightLevel = 0;
    u8 xOffset = 0;
    u8 yOffset = 0;
    u8 width = 0;
    u8 height = 0;

    u32 bitmapExistsOffset = 0; // Not quite sure how this works
    u32 vertexDataOffset = 0; // Not quite sure how this works
};

struct MH2O_ChunkAttributes
{
    u64 fishable = 0; // seems to be usable as visibility information.
    u64 deep = 0; // Might be related to fatigue area if bit set

    // Note that these are 8*8 bit masks.
};
#pragma pack(pop)
