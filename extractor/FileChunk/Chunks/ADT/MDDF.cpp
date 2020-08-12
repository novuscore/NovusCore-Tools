#include "MDDF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

#include "../../../Utils/ChunkUtils.h"

bool MDDF::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    // Doodads are not required
    if (header.size == 0)
        return true;

    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.mddf.data);
}
