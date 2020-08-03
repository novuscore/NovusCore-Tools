#include "MFOG.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MFOG::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(MFOG);
    if (num == 0)
        return true;

    wmo.mfogs.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MFOG& mfog = wmo.mfogs.emplace_back();
        if (!buffer->Get(mfog))
        {
            assert(false);
            return false;
        }
    }

    return true;
}