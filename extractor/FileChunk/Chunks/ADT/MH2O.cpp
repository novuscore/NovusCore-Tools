#include "MH2O.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MH2O::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    if (!buffer->Get(adt.mh2o))
    {
        assert(false);
        return false;
    }

    size_t extraDataSize = header.size - sizeof(MH2O);
    buffer->readData += extraDataSize;
    
    return true;
}
