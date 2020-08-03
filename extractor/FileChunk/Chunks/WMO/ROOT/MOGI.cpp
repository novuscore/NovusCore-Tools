#include "MOGI.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOGI::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(MOGI);
    if (num == 0)
        return true;

    wmo.mogis.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MOGI& mogi = wmo.mogis.emplace_back();
        if (!buffer->Get(mogi))
        {
            assert(false);
            return false;
        }
    }

    return true;
}