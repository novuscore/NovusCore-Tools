#include "MFBO.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MFBO::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    if (!buffer->Get(adt.mfbo))
    {
        assert(false);
        return false;
    }

    return true;
}
