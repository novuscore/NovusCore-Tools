#include "MOSB.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOSB::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.mosb.skyboxName = reinterpret_cast<char*>(buffer->GetReadPointer());

    buffer->readData += header.size;
    return true;
}