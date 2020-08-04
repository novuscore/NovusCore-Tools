#include "MODF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/WDT.h"
#include "../../Wrappers/ADT.h"

#include "../../../Utils/ChunkUtils.h"

bool MODF::ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    if (!buffer->Get(wdt.modf))
    {
        assert(false);
        return false;
    }

    return true;
}

bool MODF::ReadADT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.modf.data);
}