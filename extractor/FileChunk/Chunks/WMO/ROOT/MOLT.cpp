#include "MOLT.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOLT::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(MOLT);
    if (num == 0)
        return true;

    wmo.molts.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MOLT& molt = wmo.molts.emplace_back();
        if (!buffer->Get(molt))
        {
            assert(false);
            return false;
        }
    }

    return true;
}