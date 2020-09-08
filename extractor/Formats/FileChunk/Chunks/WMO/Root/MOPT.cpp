#include "MOPT.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOPT::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    // Portals are not required
    if (header.size == 0)
        return true;

    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.mopt.data);
}