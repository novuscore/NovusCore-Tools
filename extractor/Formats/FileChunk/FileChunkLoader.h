#pragma once
#include <robin_hood.h>
#include <Utils/ByteBuffer.h>
#include "Chunks/FileChunkHeader.h"

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
class FileChunkLoader
{
public:
    FileChunkLoader();

    bool LoadWdt(std::shared_ptr<Bytebuffer>& buffer, Wdt::Wdt& wdt);
    bool LoadAdt(std::shared_ptr<Bytebuffer>& buffer, const Wdt::Wdt& wdt, Adt::Adt& adt);
    bool LoadMapObjectRoot(std::shared_ptr<Bytebuffer>& buffer, WMO_ROOT& wmo);
    bool LoadMapObjectGroup(std::shared_ptr<Bytebuffer>& buffer, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);

private:
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const FileChunkHeader& header, Wdt::Wdt&)>> _wdtChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const FileChunkHeader& header, const Wdt::Wdt&, Adt::Adt&)>> _adtChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const FileChunkHeader& header, WMO_ROOT&)>> _wmoRootChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const FileChunkHeader& header, const WMO_ROOT&, WMO_OBJECT&)>> _wmoObjectChunkToFunction;
};