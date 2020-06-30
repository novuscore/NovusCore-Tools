#include "MODF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/WDT.h"
#include "../../Wrappers/ADT.h"

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
    size_t num = header.size / sizeof(MODF);
    if (num == 0)
        return true;

    adt.mddfs.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MODF modf;
        if (!buffer->Get(modf))
        {
            assert(false);
            return false;
        }
        
        adt.modfs.push_back(modf);
    }

    return true;
}