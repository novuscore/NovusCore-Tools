#include "MOCV.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

#include "../../../../Utils/ChunkUtils.h"

bool MOCV::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    bool hasVertexColors = wmoObject.mogp.flags & 0x4;

    MOCVData& data = wmoObject.mocv.data.emplace_back();

    data.isAlphaOnly = (!hasVertexColors) || wmoObject.mocv.data.size() > 1;

    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, data.vertexColors);
}