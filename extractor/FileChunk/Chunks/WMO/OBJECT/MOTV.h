#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct WMO_OBJECT;
struct ChunkHeader;
struct MOTV
{
    std::vector<vec2> vertexUVs;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};