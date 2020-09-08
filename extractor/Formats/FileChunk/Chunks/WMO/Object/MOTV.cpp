#include "MOTV.h"
#include "../../FileChunkHeader.h"
#include "../Root/MapObjectRoot.h"
#include "MapObjectGroup.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOTV::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    MOTVData& data = wmoObject.motv.data.emplace_back();
    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, data.vertexUVs);
}