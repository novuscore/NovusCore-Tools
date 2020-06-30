#include "MDDF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MDDF::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    size_t num = header.size / sizeof(MDDF);
    if (num == 0)
        return true;

    adt.mddfs.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MDDF mddf;
        if (!buffer->Get(mddf))
        {
            assert(false);
            return false;
        }

        adt.mddfs.push_back(mddf);
    }

    return true;
}
