#include "MOVT.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

bool MOVT::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    size_t num = header.size / sizeof(vec3);
    if (num == 0)
        return true;

    wmoObject.movt.vertexPosition.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        vec3& vertex = wmoObject.movt.vertexPosition.emplace_back();
        if (!buffer->Get(vertex))
        {
            assert(false);
            return false;
        }
    }

    return true;
}