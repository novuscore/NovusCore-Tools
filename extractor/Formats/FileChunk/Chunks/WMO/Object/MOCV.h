#pragma once
#include <Utils/ByteBuffer.h>
#include "../../FileChunkHeader.h"

#include <vector>

struct WMO_ROOT;
struct WMO_OBJECT;
struct FileChunkHeader;
struct MOCV
{
    struct MOCVData
    {
        bool isAlphaOnly = false;
        std::vector<IntColor> vertexColors;
    };

    std::vector<MOCVData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};