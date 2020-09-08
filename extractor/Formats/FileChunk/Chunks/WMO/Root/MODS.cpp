#include "MODS.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MODS::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    // Doodads are not required
    if (header.size == 0)
        return true;

    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.mods.data);
}