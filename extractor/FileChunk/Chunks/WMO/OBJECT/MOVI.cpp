#include "MOVI.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

#include "../../../../Utils/ChunkUtils.h"

bool MOVI::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, wmoObject.movi.indices);
}