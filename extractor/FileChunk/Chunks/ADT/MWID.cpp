#include "MWID.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MWID::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mwid.size = header.size;
    adt.mwid.offset = buffer->GetReadPointer();
    buffer->readData += adt.mwid.size;

    return true;
}
