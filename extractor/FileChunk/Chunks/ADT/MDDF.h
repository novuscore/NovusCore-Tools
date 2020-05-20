#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MDDFFlags
{
    Biodome = 1 << 0,
    Shrubbery = 1 << 1,
    Unk_0x4 = 1 << 2,
    Unk_0x8 = 1 << 3,
    LiquidKnown = 1 << 5,
    EntryIsFiledataId = 1 << 6,
    Unk_0x100 = 1 << 8,
};

#pragma pack(push, 1)
// Placement information for doodads Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MDDF
{
    u32 nameId = 0;
    u32 uniqueId = 0;
    vec3 position = vec3(0,0,0);
    vec3 rotation = vec3(0,0,0);
    u16 scale = 0;
    u16 flags = 0;

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
