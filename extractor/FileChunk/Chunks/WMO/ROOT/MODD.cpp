#include "MODD.h"
#include "../../ChunkHeader.h"
#include "../../../Wrappers/WMO_ROOT.h"

bool MODD::Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo)
{
    size_t num = header.size / (sizeof(MODD) - sizeof(u8)); // The nameIndex & Flags in MODD is packed together
    if (num == 0)
        return true;

    wmo.modds.reserve(num);
    for (size_t i = 0; i < num; i++)
    {
        MODD& modd = wmo.modds.emplace_back();

        u32 packedValue = 0;
        if (!buffer->GetU32(packedValue) || !buffer->Get(modd.position) || !buffer->Get(modd.rotation) ||
            !buffer->GetF32(modd.scale) || !buffer->GetU32(modd.color))
        {
            assert(false);
            return false;
        }

        modd.nameIndex = packedValue & 0xFFFFFF;
        modd.flags = (packedValue >> 24) & 0xFF;
    }

    return true;
}