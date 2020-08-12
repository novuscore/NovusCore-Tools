#include "MOMT.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

#include "../../../../Utils/ChunkUtils.h"

bool MOMT::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    // Materials are not required (for wmo colliders)
    if (header.size == 0)
        return true;

    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, wmo.momt.data);
}