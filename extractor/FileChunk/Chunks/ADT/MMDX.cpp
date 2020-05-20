#include "MMDX.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MMDX::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    adt.mmdx.size = header.size;
    adt.mmdx.filenames = buffer->GetReadPointer();
    buffer->ReadData += adt.mmdx.size;

    return true;
}
