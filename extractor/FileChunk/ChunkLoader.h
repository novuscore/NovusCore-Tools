#pragma once
#include <robin_hood.h>
#include <Utils/ByteBuffer.h>
#include "Chunks/ChunkHeader.h"

struct WDT;
struct ADT;
class ChunkLoader
{
public:
    ChunkLoader();
    bool LoadWDT(std::shared_ptr<ByteBuffer>& buffer, WDT& wdt);
    bool LoadADT(std::shared_ptr<ByteBuffer>& buffer, const WDT& wdt, ADT& adt);

private:
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<ByteBuffer>&, const ChunkHeader& header, WDT&)>> _wdtChunkToFunction;
    robin_hood::unordered_map<u32, std::function<bool(std::shared_ptr<ByteBuffer>&, const ChunkHeader& header, const WDT&, ADT&)>> _adtChunkToFunction;
};