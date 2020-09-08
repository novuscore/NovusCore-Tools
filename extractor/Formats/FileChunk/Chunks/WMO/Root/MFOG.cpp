#include "MFOG.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MFOG::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.mfog.data);
}