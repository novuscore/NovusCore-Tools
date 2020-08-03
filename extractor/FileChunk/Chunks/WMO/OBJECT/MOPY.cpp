#include "MOPY.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

bool MOPY::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    size_t num = header.size / sizeof(MOPY);
    if (num == 0)
        return true;

    wmoObject.mopys.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MOPY& mopy = wmoObject.mopys.emplace_back();
        if (!buffer->Get(mopy))
        {
            assert(false);
            return false;
        }
    }

    return true;
}