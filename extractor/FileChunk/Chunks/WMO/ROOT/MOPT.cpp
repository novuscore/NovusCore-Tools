#include "MOPT.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOPT::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(MOPT);
    if (num == 0)
        return true;

    wmo.mopts.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MOPT& mopt = wmo.mopts.emplace_back();
        if (!buffer->Get(mopt))
        {
            assert(false);
            return false;
        }
    }

    return true;
}