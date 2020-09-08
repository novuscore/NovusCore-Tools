#include "MOGI.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOGI::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.mogi.data);
}