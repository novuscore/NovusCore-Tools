#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>
#include "../../ChunkHeader.h"

struct WMO_ROOT;
struct WMO_OBJECT;
struct ChunkHeader;
struct MOCV
{
    struct MOCVData
    {
        bool isAlphaOnly = false;
        std::vector<IntColor> vertexColors;
    };

    std::vector<MOCVData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};