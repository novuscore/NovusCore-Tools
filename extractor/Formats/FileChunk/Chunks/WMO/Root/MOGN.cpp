#include "MOGN.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

bool MOGN::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    wmo.mogn.size = header.size;
    wmo.mogn.groupNames = buffer->GetReadPointer();

    buffer->readData += header.size;
    return true;
}