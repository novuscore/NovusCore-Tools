#include "ChunkLoader.h"
#include "Wrappers/WDT.h"
#include "Wrappers/ADT.h"
#include <cassert>

ChunkLoader::ChunkLoader()
{
    _wdtChunkToFunction =
    {
        { static_cast<u32>('MVER'), MVER::ReadWDT },
        { static_cast<u32>('MPHD'), MPHD::Read },
        { static_cast<u32>('MAIN'), MAIN::Read },
        { static_cast<u32>('MWMO'), MWMO::ReadWDT },
        { static_cast<u32>('MODF'), MODF::ReadWDT }
    };
    _adtChunkToFunction =
    {
        { static_cast<u32>('MVER'), MVER::ReadADT },
        { static_cast<u32>('MHDR'), MHDR::Read },
        { static_cast<u32>('MCIN'), MCIN::Read },
        { static_cast<u32>('MTEX'), MTEX::Read },
        { static_cast<u32>('MMDX'), MMDX::Read },
        { static_cast<u32>('MMID'), MMID::Read },
        { static_cast<u32>('MWMO'), MWMO::ReadADT },
        { static_cast<u32>('MWID'), MWID::Read },
        { static_cast<u32>('MDDF'), MDDF::Read },
        { static_cast<u32>('MODF'), MODF::ReadADT },
        { static_cast<u32>('MFBO'), MFBO::Read },
        { static_cast<u32>('MH2O'), MH2O::Read },
        { static_cast<u32>('MTXF'), MTXF::Read },
        { static_cast<u32>('MCNK'), MCNK::Read }
    };
}

bool ChunkLoader::LoadWDT(std::shared_ptr<ByteBuffer>& buffer, WDT& wdt)
{
    ChunkHeader header;
    do
    {
        if (!buffer->Get<ChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _wdtChunkToFunction.find(header.token);
        if (itr != _wdtChunkToFunction.end())
        {
            if (!itr->second(buffer, header, wdt))
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }

    } while (buffer->ReadData != buffer->Size);


    return true;
}
bool ChunkLoader::LoadADT(std::shared_ptr<ByteBuffer>& buffer, const WDT& wdt, ADT& adt)
{
    ChunkHeader header;
    do
    {
        if (!buffer->Get<ChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _adtChunkToFunction.find(header.token);
        if (itr != _adtChunkToFunction.end())
        {
            if (!itr->second(buffer, header, wdt, adt))
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }

    } while (buffer->ReadData != buffer->Size);


    return true;
}
