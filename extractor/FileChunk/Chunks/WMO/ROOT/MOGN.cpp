#include "MOGN.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOGN::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.mogn.size = header.size;
    wmo.mogn.groupNames = buffer->GetReadPointer();

    buffer->readData += header.size;
    return true;
}