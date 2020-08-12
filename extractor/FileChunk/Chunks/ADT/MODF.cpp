#include "MODF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/WDT.h"
#include "../../Wrappers/ADT.h"

#include "../../../Utils/ChunkUtils.h"

bool MODF::ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WDT& wdt)
{
    MODF::MODFData& data = wdt.modf.data.emplace_back();
    if (!buffer->Get(data))
    {
        assert(false);
        return false;
    }

    return true;
}

bool MODF::ReadADT(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    // WMOs are not required
    if (header.size == 0)
        return true;

    return ChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.modf.data);
}