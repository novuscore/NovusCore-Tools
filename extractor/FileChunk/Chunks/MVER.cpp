#include "MVER.h"
#include "../Wrappers/WDT.h"
#include "../Wrappers/ADT.h"
#include "../Wrappers/WMO_ROOT.h"
#include "../Wrappers/WMO_OBJECT.h"

bool MVER::ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    if (!buffer->Get(wdt.mver))
    {
        assert(false);
        return false;
    }

    return true;
}
bool MVER::ReadADT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    if (!buffer->Get(adt.mver))
    {
        assert(false);
        return false;
    }

    return true;
}

bool MVER::ReadWMO_ROOT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    if (!buffer->Get(wmo.mver))
    {
        assert(false);
        return false;
    }

    return true;
}
bool MVER::ReadWMO_OBJECT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    if (!buffer->Get(wmoObject.mver))
    {
        assert(false);
        return false;
    }

    return true;
}