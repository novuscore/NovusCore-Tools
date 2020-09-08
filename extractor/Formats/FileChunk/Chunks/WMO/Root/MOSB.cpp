#include "MOSB.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

bool MOSB::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.mosb.skyboxName = reinterpret_cast<char*>(buffer->GetReadPointer());

    buffer->readData += header.size;
    return true;
}