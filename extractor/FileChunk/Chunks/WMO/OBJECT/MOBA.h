#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct WMO_OBJECT;
struct ChunkHeader;
struct MOBA
{
    struct MOBAData
    {
        i16 bx, by, bz; // Bounding Box (culling)
        i16 tx, ty, tz;

        u32 startIndex; // This is an index into MOVI
        u16 indexCount;
        u16 firstVertex; // Start Vertex
        u16 lastVertex; // Last Vertex
        u8 unk; // Flag?

        u8 materialId; // This is an index into MOMT;
    };

    std::vector<MOBAData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};