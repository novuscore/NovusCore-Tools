#include "ChunkLoader.h"
#include "Wrappers/WDT.h"
#include "Wrappers/ADT.h"
#include "Wrappers/WMO_ROOT.h"
#include "Wrappers/WMO_OBJECT.h"
#include <cassert>

#include <tracy/Tracy.hpp>

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
    _wmoRootChunkToFunction =
    {
        { static_cast<u32>('MVER'), MVER::ReadWMO_ROOT },
        { static_cast<u32>('MOHD'), MOHD::Read },
        { static_cast<u32>('MOTX'), MOTX::Read },
        { static_cast<u32>('MOMT'), MOMT::Read },
        { static_cast<u32>('MOGN'), MOGN::Read },
        { static_cast<u32>('MOGI'), MOGI::Read },
        { static_cast<u32>('MOSB'), MOSB::Read },
        { static_cast<u32>('MOPV'), MOPV::Read },
        { static_cast<u32>('MOPT'), MOPT::Read },
        { static_cast<u32>('MOPR'), MOPR::Read },
        { static_cast<u32>('MOLT'), MOLT::Read },
        { static_cast<u32>('MODS'), MODS::Read },
        { static_cast<u32>('MODN'), MODN::Read },
        { static_cast<u32>('MODD'), MODD::Read },
        { static_cast<u32>('MFOG'), MFOG::Read },
        { static_cast<u32>('MCVP'), MCVP::Read },

        // The following Chunks are seen in the (3.3.5a client, but with no size)
        { static_cast<u32>('MOVV'), nullptr },
        { static_cast<u32>('MOVB'), nullptr }
    };
    _wmoObjectChunkToFunction =
    {
        { static_cast<u32>('MVER'), MVER::ReadWMO_OBJECT },
        { static_cast<u32>('MOGP'), MOGP::Read },
        { static_cast<u32>('MOPY'), MOPY::Read },
        { static_cast<u32>('MOVI'), MOVI::Read },
        { static_cast<u32>('MOVT'), MOVT::Read },
        { static_cast<u32>('MONR'), MONR::Read },
        { static_cast<u32>('MOTV'), MOTV::Read },
        { static_cast<u32>('MOBA'), MOBA::Read },
    };
}

bool ChunkLoader::LoadWDT(std::shared_ptr<Bytebuffer>& buffer, WDT& wdt)
{
    ZoneScoped;

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
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wdt))
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }

    } while (buffer->readData != buffer->size);


    return true;
}
bool ChunkLoader::LoadADT(std::shared_ptr<Bytebuffer>& buffer, const WDT& wdt, ADT& adt)
{
    ZoneScoped;

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
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wdt, adt))
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }

    } while (buffer->readData != buffer->size);


    return true;
}

bool ChunkLoader::LoadWMO_ROOT(std::shared_ptr<Bytebuffer>& buffer, WMO_ROOT& wmo)
{
    ZoneScoped;

    ChunkHeader header;
    do
    {
        if (!buffer->Get<ChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _wmoRootChunkToFunction.find(header.token);
        if (itr != _wmoRootChunkToFunction.end())
        {
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wmo))
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }

    } while (buffer->readData != buffer->size);


    return true;
}

bool ChunkLoader::LoadWMO_OBJECT(std::shared_ptr<Bytebuffer>& buffer, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    ZoneScoped;

    ChunkHeader header;
    do
    {
        if (!buffer->Get<ChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _wmoObjectChunkToFunction.find(header.token);
        if (itr != _wmoObjectChunkToFunction.end())
        {
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wmoRoot, wmoObject))
            {
                assert(false);
            }

            // TODO: Support all Optional Chunks
            if (header.token == 'MOBA')
                break;
        }
        else
        {
            assert(false);
        }

    } while (buffer->readData != buffer->size);


    return true;
}
