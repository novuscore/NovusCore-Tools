#include "MOTV.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

#include "../../../../Utils/ChunkUtils.h"

bool MOTV::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    MOTVData& data = wmoObject.motv.data.emplace_back();
    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, data.vertexUVs);
}