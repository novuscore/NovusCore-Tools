#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct ChunkHeader;
struct MOGI
{
    struct MOGIData
    {
        u32 flags = 0;
        vec3 boundingBoxMin; // Axis Aligned Box Min
        vec3 boundingBoxMax; // Axis Aligned box Max
        i32 nameOffset = 0; // Offset into MOGN (value is -1 if no name is set)
    };

    std::vector<MOGIData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};