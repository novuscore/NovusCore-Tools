#include "MMID.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MMID::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mmid.size = header.size;
    adt.mmid.offsets = buffer->GetReadPointer();
    buffer->readData += adt.mmid.size;

    return true;
}
