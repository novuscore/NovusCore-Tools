#include "MWID.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MWID::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mwid.size = header.size;
    adt.mwid.offset = buffer->GetReadPointer();
    buffer->ReadData += adt.mwid.size;

    return true;
}
