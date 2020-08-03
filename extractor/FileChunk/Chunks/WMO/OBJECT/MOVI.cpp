#include "MOVI.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"
#include "../../../Wrappers/WMO_OBJECT.h"

bool MOVI::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    size_t num = header.size / sizeof(u16);
    if (num == 0)
        return true;

    wmoObject.movi.indices.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        u16& num = wmoObject.movi.indices.emplace_back();
        if (!buffer->GetU16(num))
        {
            assert(false);
            return false;
        }
    }

    return true;
}