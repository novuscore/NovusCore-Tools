#include "MMDX.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MMDX::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mmdx.size = header.size;
    adt.mmdx.filenames = buffer->GetReadPointer();
    buffer->readData += adt.mmdx.size;

    return true;
}
