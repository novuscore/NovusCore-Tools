#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct WMO_ROOT;
struct ChunkHeader;
struct MOPT
{
    u16 startVertex = 0;
    u16 count = 0;

    vec3 normal;
    f32 distance = 0;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};