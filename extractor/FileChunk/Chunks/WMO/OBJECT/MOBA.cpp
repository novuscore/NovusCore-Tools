#include "MOBA.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

bool MOBA::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    size_t num = header.size / sizeof(MOBA);
    if (num == 0)
        return true;

    wmoObject.mobas.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MOBA& moba = wmoObject.mobas.emplace_back();
        if (!buffer->Get(moba))
        {
            assert(false);
            return false;
        }
    }

    return true;
}