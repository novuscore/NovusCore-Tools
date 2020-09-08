#include "MOLT.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOLT::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    // Lights are not required
    if (header.size == 0)
        return true;

    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.molt.data);
}