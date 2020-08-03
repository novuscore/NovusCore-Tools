#include "MOHD.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOHD::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    if (!buffer->Get(wmo.mohd))
    {
        assert(false);
        return false;
    }

    return true;
}