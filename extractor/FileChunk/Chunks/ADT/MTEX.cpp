#include "MTEX.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MTEX::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mtex.size = header.size;
    adt.mtex.filenames = buffer->GetReadPointer();
    buffer->readData += adt.mtex.size;

    return true;
}
