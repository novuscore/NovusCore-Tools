#include "MVER.h"
#include "WDT/Wdt.h"
#include "ADT/Adt.h"
#include "WMO/Root/MapObjectRoot.h"
#include "WMO/Object/MapObjectGroup.h"

bool Mver::ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt::Wdt& wdt)
{
    if (!buffer->Get(wdt.mver))
    {
        assert(false);
        return false;
    }

    return true;
}
bool Mver::ReadADT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt::Adt& adt)
{
    if (!buffer->Get(adt.mver))
    {
        assert(false);
        return false;
    }

    return true;
}

bool Mver::ReadWMO_ROOT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    if (!buffer->Get(wmo.mver))
    {
        assert(false);
        return false;
    }

    return true;
}
bool Mver::ReadWMO_OBJECT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    if (!buffer->Get(wmoObject.mver))
    {
        assert(false);
        return false;
    }

    return true;
}