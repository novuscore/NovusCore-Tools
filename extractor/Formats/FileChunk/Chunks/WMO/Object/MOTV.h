#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct WMO_OBJECT;
struct FileChunkHeader;
struct MOTV
{
    struct MOTVData
    {
        std::vector<vec2> vertexUVs;
    };

    std::vector<MOTVData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};