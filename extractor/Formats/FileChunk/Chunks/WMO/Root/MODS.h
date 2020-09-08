#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct FileChunkHeader;
struct MODS
{
    struct MODSData
    {
        char name[20] = { 0 };
        u32 startIndex = 0; // Index into MODD
        u32 count = 0;
        char pad[4] = { 0 };
    };

    std::vector<MODSData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};