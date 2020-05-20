#include "MWMO.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/WDT.h"
#include "../../Wrappers/ADT.h"

bool MWMO::ReadWDT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    wdt.mwmo.size = header.size;
    wdt.mwmo.filenames = buffer->GetReadPointer();
    buffer->ReadData += wdt.mwmo.size;

    return true;
}

bool MWMO::ReadADT(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mwmo.size = header.size;
    adt.mwmo.filenames = buffer->GetReadPointer();
    buffer->ReadData += adt.mwmo.size;

    return true;
}