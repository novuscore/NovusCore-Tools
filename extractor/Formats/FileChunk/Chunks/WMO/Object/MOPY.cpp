#include "MOPY.h"
#include "../../FileChunkHeader.h"
#include "../Root/MapObjectRoot.h"
#include "MapObjectGroup.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOPY::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, wmoObject.mopy.data);
}