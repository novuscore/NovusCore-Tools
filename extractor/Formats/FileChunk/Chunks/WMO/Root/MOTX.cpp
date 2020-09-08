#include "MOTX.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

bool MOTX::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.motx.size = header.size;
    wmo.motx.textureNames = buffer->GetReadPointer();

    buffer->readData += header.size;
    return true;
}