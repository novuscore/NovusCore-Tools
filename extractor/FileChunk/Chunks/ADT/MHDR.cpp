#include "MHDR.h"
#include "../../Wrappers/ADT.h"

bool MHDR::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    u32 parentOffset = static_cast<u32>(buffer->ReadData);
    if (!buffer->Get(adt.mhdr))
    {
        assert(false);
        return false;
    }

    // Convert offsets to absolute
    if (adt.mhdr.mcin)
        adt.mhdr.mcin += parentOffset;

    if (adt.mhdr.mtex)
        adt.mhdr.mtex += parentOffset;

    if (adt.mhdr.mmdx)
        adt.mhdr.mmdx += parentOffset;

    if (adt.mhdr.mmid)
        adt.mhdr.mmid += parentOffset;

    if (adt.mhdr.mwmo)
        adt.mhdr.mwmo += parentOffset;

    if (adt.mhdr.mwid)
        adt.mhdr.mwid += parentOffset;

    if (adt.mhdr.mddf)
        adt.mhdr.mddf += parentOffset;

    if (adt.mhdr.modf)
        adt.mhdr.modf += parentOffset;

    if (adt.mhdr.mfbo)
        adt.mhdr.mfbo += parentOffset;

    if (adt.mhdr.mh2o)
        adt.mhdr.mh2o += parentOffset;

    if (adt.mhdr.mtxf)
        adt.mhdr.mtxf += parentOffset;

    return true;
}
