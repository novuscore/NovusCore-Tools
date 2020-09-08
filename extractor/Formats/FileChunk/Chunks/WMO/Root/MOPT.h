#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct FileChunkHeader;
struct MOPT
{
    struct MOPTData
    {
        u16 startVertex = 0;
        u16 count = 0;

        vec3 normal;
        f32 distance = 0;
    };

    std::vector<MOPTData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};