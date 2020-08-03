#include "MOPR.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOPR::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(MOPR);
    if (num == 0)
        return true;

    wmo.moprs.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MOPR& mopr = wmo.moprs.emplace_back();
        if (!buffer->Get(mopr))
        {
            assert(false);
            return false;
        }
    }

    return true;
}