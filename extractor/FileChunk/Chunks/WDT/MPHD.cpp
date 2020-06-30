#include "MPHD.h"
#include "../../Wrappers/WDT.h"

bool MPHD::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    if (!buffer->Get(wdt.mphd))
    {
        assert(false);
        return false;
    }

    wdt.hasBigAlpha = (wdt.mphd.flags & static_cast<u32>(MPHDFlags::AdtHasBigAlpha));
    return true;
}
