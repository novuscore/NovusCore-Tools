#include "MOBA.h"
#include "../../FileChunkHeader.h"
#include "../Root/MapObjectRoot.h"
#include "MapObjectGroup.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOBA::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    // Render Batches are not required
    if (header.size == 0)
        return true;

    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmoObject.moba.data);
}