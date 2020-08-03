#include "MODN.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MODN::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.modn.size = header.size;
    wmo.modn.doodadNames = buffer->GetReadPointer();

    buffer->readData += header.size;
    return true;
}