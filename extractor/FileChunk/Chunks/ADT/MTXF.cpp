#include "MTXF.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/ADT.h"

bool MTXF::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    size_t num = header.size / sizeof(MTXF);
    if (num == 0)
        return true;

    adt.mddfs.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MTXF mtxf;
        if (!buffer->Get(mtxf))
        {
            assert(false);
            return false;
        }
        
        adt.mtxfs.push_back(mtxf);
    }

    return true;
}
