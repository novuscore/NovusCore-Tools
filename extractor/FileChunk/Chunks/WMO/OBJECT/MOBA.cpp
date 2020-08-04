#include "MOBA.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

#include "../../../../Utils/ChunkUtils.h"

bool MOBA::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    // Render Batches are not required
    if (header.size == 0)
        return true;

    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, wmoObject.moba.data);
}