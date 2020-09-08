#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct WMO_ROOT;
struct FileChunkHeader;
struct MOTX
{
    u32 size = 0;
    u8* textureNames = nullptr;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};