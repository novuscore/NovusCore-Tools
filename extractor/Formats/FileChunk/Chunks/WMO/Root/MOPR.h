#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct FileChunkHeader;
struct MOPR
{
    struct MOPRData
    {
        u16 portalIndex = 0; // Index into MOPT
        u16 groupIndex = 0; // index into MOGT
        i16 side = 0;
        u16 filler = 0;
    };

    std::vector<MOPRData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};