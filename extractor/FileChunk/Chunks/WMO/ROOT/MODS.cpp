#include "MODS.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MODS::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(MODS);
    if (num == 0)
        return true;

    wmo.modss.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MODS& mods = wmo.modss.emplace_back();
        if (!buffer->Get(mods))
        {
            assert(false);
            return false;
        }
    }

    return true;
}