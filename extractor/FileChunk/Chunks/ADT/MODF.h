#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

enum class MODFFlags : u16
{
    Destroyable = 1 << 0,
    UseLod = 1 << 1,             // WoD(?)+: also load _LOD1.WMO for use dependent on distance
    HasScale = 1 << 2,           // Legion+: if this flag is set then use scale = scale / 1024, otherwise scale is 1.0
    EntryIsFiledataId = 1 << 3   // Legion+: nameId is a file data id to directly load
};

#pragma pack(push, 1)
// Placement information for WMOs Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MODF
{
    struct MODFData
    {
        u32 nameId = 0; // Reference to MWID chunk
        u32 uniqueId = 0; // This ID should be unique
        vec3 position = vec3(0, 0, 0);
        vec3 rotation = vec3(0, 0, 0);

        // Bounding Box
        vec3 min = vec3(0, 0, 0);
        vec3 max = vec3(0, 0, 0);

        u16 flags = 0;
        u16 doodadSet = 0;
        u16 nameSet = 0;
        u16 scale = 0;
    };

    std::vector<MODFData> data;

    static bool ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WDT& wdt);
    static bool ReadADT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
