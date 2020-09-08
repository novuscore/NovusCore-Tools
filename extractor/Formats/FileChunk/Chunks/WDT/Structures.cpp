#include "Structures.h"
#include "Wdt.h"

namespace Wdt
{
    bool Main::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt& wdt)
    {
        if (!buffer->Get(wdt.main))
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool Mphd::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt& wdt)
    {
        if (!buffer->Get(wdt.mphd))
        {
            assert(false);
            return false;
        }

        wdt.hasBigAlpha = wdt.mphd.flags.AdtHasBigAlpha;
        return true;
    }
}