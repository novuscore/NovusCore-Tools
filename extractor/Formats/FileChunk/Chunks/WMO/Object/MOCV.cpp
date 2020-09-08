#include "MOCV.h"
#include "../../FileChunkHeader.h"
#include "MapObjectGroup.h"
#include "../Root/MapObjectRoot.h"

#include "../../../../../Utils/FileChunkUtils.h"

bool MOCV::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    bool hasVertexColors = wmoObject.mogp.flags & 0x4;

    MOCVData& data = wmoObject.mocv.data.emplace_back();

    data.isAlphaOnly = (!hasVertexColors) || wmoObject.mocv.data.size() > 1;

    return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, data.vertexColors);
}