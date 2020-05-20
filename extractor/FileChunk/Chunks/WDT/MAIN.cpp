#include "MAIN.h"
#include "../../Wrappers/WDT.h"

bool MAIN::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    if (!buffer->Get(wdt.main))
    {
        assert(false);
        return false;
    }

    return true;
}