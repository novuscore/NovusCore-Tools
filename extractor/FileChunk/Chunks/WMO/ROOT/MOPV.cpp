#include "MOPV.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MOPV::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / sizeof(vec3);
    if (num == 0)
        return true;

    wmo.mopv.portalVertexList.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        vec3& vec = wmo.mopv.portalVertexList.emplace_back();
        if (!buffer->Get(vec))
        {
            assert(false);
            return false;
        }
    }

    return true;
}