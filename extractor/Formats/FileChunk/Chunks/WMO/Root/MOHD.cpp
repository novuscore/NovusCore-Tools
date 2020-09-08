#include "MOHD.h"
#include "../../FileChunkHeader.h"
#include "MapObjectRoot.h"

bool MOHD::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo)
{
    if (!buffer->Get(wmo.mohd))
    {
        assert(false);
        return false;
    }

    return true;
}