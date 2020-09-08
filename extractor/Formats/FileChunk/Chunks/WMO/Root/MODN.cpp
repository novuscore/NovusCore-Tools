#include "MODN.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

bool MODN::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    // Doodads are not required
    if (header.size == 0)
        return true;

    wmo.modn.size = header.size;
    wmo.modn.doodadNames = buffer->GetReadPointer();

    buffer->readData += header.size;
    return true;
}