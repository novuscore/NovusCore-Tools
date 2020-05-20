#include "MVER.h"
#include "../Wrappers/WDT.h"
#include "../Wrappers/ADT.h"

bool MVER::ReadWDT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    if (!buffer->Get(wdt.mver))
    {
        assert(false);
        return false;
    }

    return true;
}
bool MVER::ReadADT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    if (!buffer->Get(adt.mver))
    {
        assert(false);
        return false;
    }

    return true;
}