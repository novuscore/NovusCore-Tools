#include "MCVP.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MCVP::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(ConvexVolumePlane);
    if (num == 0)
        return true;

    wmo.mcvp.convexVolumePlanes.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        ConvexVolumePlane& plane = wmo.mcvp.convexVolumePlanes.emplace_back();
        if (!buffer->Get(plane))
        {
            assert(false);
            return false;
        }
    }

    return true;
}