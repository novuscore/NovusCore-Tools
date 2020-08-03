#include "MOTV.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

bool MOTV::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    size_t num = header.size / sizeof(vec2);
    if (num == 0)
        return true;

    wmoObject.motv.vertexUVs.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        vec2& vertexUV = wmoObject.motv.vertexUVs.emplace_back();
        if (!buffer->Get(vertexUV))
        {
            assert(false);
            return false;
        }
    }

    return true;
}