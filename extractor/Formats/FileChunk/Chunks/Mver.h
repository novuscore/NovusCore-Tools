#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Version Chunk

namespace Wdt
{
    struct Wdt;
}

namespace Adt
{
    struct Adt;
}
struct WMO_ROOT;
struct WMO_OBJECT;
struct FileChunkHeader;
struct Mver
{
    u32 version;

    static bool ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt::Wdt& wdt);
    static bool ReadADT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt::Adt& adt);
    static bool ReadWMO_ROOT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
    static bool ReadWMO_OBJECT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};
#pragma pack(pop)