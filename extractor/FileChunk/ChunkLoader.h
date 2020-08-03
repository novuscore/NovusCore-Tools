#pragma once
#include <robin_hood.h>
#include <Utils/ByteBuffer.h>
#include "Chunks/ChunkHeader.h"

struct WDT;
struct ADT;
struct WMO_ROOT;
struct WMO_OBJECT;
class ChunkLoader
{
public:
    ChunkLoader();
    bool LoadWDT(std::shared_ptr<Bytebuffer>& buffer, WDT& wdt);
    bool LoadADT(std::shared_ptr<Bytebuffer>& buffer, const WDT& wdt, ADT& adt);
    bool LoadWMO_ROOT(std::shared_ptr<Bytebuffer>& buffer, WMO_ROOT& wmo);
    bool LoadWMO_OBJECT(std::shared_ptr<Bytebuffer>& buffer, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);

private:
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const ChunkHeader& header, WDT&)>> _wdtChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const ChunkHeader& header, const WDT&, ADT&)>> _adtChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const ChunkHeader& header, WMO_ROOT&)>> _wmoRootChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<Bytebuffer>&, const ChunkHeader& header, const WMO_ROOT&, WMO_OBJECT&)>> _wmoObjectChunkToFunction;
};