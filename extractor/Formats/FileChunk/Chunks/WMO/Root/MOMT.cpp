#include "MOMT.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOMT::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    // Materials are not required (for wmo colliders)
    if (header.size == 0)
        return true;

    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.momt.data);
}