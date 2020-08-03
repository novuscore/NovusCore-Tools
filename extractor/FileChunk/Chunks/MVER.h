#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Version Chunk
struct WDT;
struct ADT;
struct WMO_ROOT;
struct WMO_OBJECT;
struct ChunkHeader;
struct MVER
{
    u32 version;

    static bool ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WDT& wdt);
    static bool ReadADT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
    static bool ReadWMO_ROOT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
    static bool ReadWMO_OBJECT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};
#pragma pack(pop)