#include "MCIN.h"
#include "../../Wrappers/ADT.h"

bool MCIN::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    if (!buffer->Get(adt.mcin))
    {
        assert(false);
        return false;
    }

    return true;
}
