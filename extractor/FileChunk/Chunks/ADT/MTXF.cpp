#include "MTXF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

#include "../../../Utils/ChunkUtils.h"

bool MTXF::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.mtxf.data);
}
