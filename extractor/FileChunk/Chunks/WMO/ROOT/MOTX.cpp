#include "MOTX.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOTX::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.motx.size = header.size;
    wmo.motx.textureNames = buffer->GetReadPointer();

    buffer->readData += header.size;
    return true;
}