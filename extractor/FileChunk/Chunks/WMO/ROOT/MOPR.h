#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct WMO_ROOT;
struct ChunkHeader;
struct MOPR
{
    u16 portalIndex = 0; // Index into MOPT
    u16 groupIndex = 0; // index into MOGT
    i16 side = 0;
    u16 filler = 0;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};