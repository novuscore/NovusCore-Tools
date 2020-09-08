#include "MOGP.h"
#include "../../FileChunkHeader.h"
#include "../Root/MapObjectRoot.h"
#include "MapObjectGroup.h"

bool MOGP::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    if (!buffer->Get(wmoObject.mogp))
    {
        assert(false);
        return false;
    }

    return true;
}