#include "FileChunkLoader.h"
#include "Chunks/WDT/Wdt.h"
#include "Chunks/ADT/Adt.h"
#include "Chunks/WMO/Root/MapObjectRoot.h"
#include "Chunks/WMO/Object/MapObjectGroup.h"
#include <cassert>

#include <tracy/Tracy.hpp>

FileChunkLoader::FileChunkLoader()
{
    _wdtChunkToFunction =
    {
        { static_cast<u32>('MVER'), Mver::ReadWDT },
        { static_cast<u32>('MPHD'), Wdt::Mphd::Read },
        { static_cast<u32>('MAIN'), Wdt::Main::Read },
        { static_cast<u32>('MWMO'), Adt::Mwmo::ReadWDT },
        { static_cast<u32>('MODF'), Adt::Modf::ReadWDT }
    };

    _adtChunkToFunction =
    {
        { static_cast<u32>('MVER'), Mver::ReadADT },
        { static_cast<u32>('MHDR'), Adt::Mhdr::Read },
        { static_cast<u32>('MCIN'), Adt::Mcin::Read },
        { static_cast<u32>('MTEX'), Adt::Mtex::Read },
        { static_cast<u32>('MMDX'), Adt::Mmdx::Read },
        { static_cast<u32>('MMID'), Adt::Mmid::Read },
        { static_cast<u32>('MWMO'), Adt::Mwmo::ReadADT },
        { static_cast<u32>('MWID'), Adt::Mwid::Read },
        { static_cast<u32>('MDDF'), Adt::Mddf::Read },
        { static_cast<u32>('MODF'), Adt::Modf::ReadADT },
        { static_cast<u32>('MFBO'), Adt::Mfbo::Read },
        { static_cast<u32>('MH2O'), Adt::Mh2o::Read },
        { static_cast<u32>('MTXF'), Adt::Mtxf::Read },
        { static_cast<u32>('MCNK'), Adt::Mcnk::Read }
    };

    _wmoRootChunkToFunction =
    {
        { static_cast<u32>('MVER'), Mver::ReadWMO_ROOT },
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
        { static_cast<u32>('MVER'), Mver::ReadWMO_OBJECT },
        { static_cast<u32>('MOGP'), MOGP::Read },
        { static_cast<u32>('MOPY'), MOPY::Read },
        { static_cast<u32>('MOVI'), MOVI::Read },
        { static_cast<u32>('MOVT'), MOVT::Read },
        { static_cast<u32>('MONR'), MONR::Read },
        { static_cast<u32>('MOTV'), MOTV::Read },
        { static_cast<u32>('MOBA'), MOBA::Read },
        { static_cast<u32>('MOCV'), MOCV::Read },

        // Implement these later
        { static_cast<u32>('MOBS'), nullptr },
        { static_cast<u32>('MOLR'), nullptr },
        { static_cast<u32>('MODR'), nullptr },
        { static_cast<u32>('MOBN'), nullptr },
        { static_cast<u32>('MOBR'), nullptr },
        { static_cast<u32>('MPBV'), nullptr },
        { static_cast<u32>('MPBP'), nullptr },
        { static_cast<u32>('MPBI'), nullptr },
        { static_cast<u32>('MPBG'), nullptr },
        { static_cast<u32>('MLIQ'), nullptr },
        { static_cast<u32>('MORI'), nullptr },
        { static_cast<u32>('MORB'), nullptr }

    };
}

bool FileChunkLoader::LoadWdt(std::shared_ptr<Bytebuffer>& buffer, Wdt::Wdt& wdt)
{
    ZoneScoped;

    FileChunkHeader header;
    do
    {
        if (!buffer->Get<FileChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _wdtChunkToFunction.find(header.token);
        assert(itr != _wdtChunkToFunction.end());

        if (itr != _wdtChunkToFunction.end())
        {
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wdt))
            {
                assert(false);
            }
        }

    } while (buffer->readData != buffer->size);

    return true;
}
bool FileChunkLoader::LoadAdt(std::shared_ptr<Bytebuffer>& buffer, const Wdt::Wdt& wdt, Adt::Adt& adt)
{
    ZoneScoped;

    FileChunkHeader header;
    do
    {
        if (!buffer->Get<FileChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _adtChunkToFunction.find(header.token);
        assert(itr != _adtChunkToFunction.end());

        if (itr != _adtChunkToFunction.end())
        {
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wdt, adt))
            {
                assert(false);
            }
        }

    } while (buffer->readData != buffer->size);

    return true;
}

bool FileChunkLoader::LoadMapObjectRoot(std::shared_ptr<Bytebuffer>& buffer, WMO_ROOT& wmo)
{
    ZoneScoped;

    FileChunkHeader header;
    do
    {
        if (!buffer->Get<FileChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _wmoRootChunkToFunction.find(header.token);
        assert(itr != _wmoRootChunkToFunction.end());

        if (itr != _wmoRootChunkToFunction.end())
        {
            if (itr->second == nullptr)
                continue;

            if (!itr->second(buffer, header, wmo))
            {
                assert(false);
            }
        }

    } while (buffer->readData != buffer->size);

    return true;
}

bool FileChunkLoader::LoadMapObjectGroup(std::shared_ptr<Bytebuffer>& buffer, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject)
{
    ZoneScoped;

    FileChunkHeader header;
    do
    {
        if (!buffer->Get<FileChunkHeader>(header))
        {
            assert(false);
            return false;
        }

        auto itr = _wmoObjectChunkToFunction.find(header.token);
        assert(itr != _wmoObjectChunkToFunction.end());

        if (itr != _wmoObjectChunkToFunction.end())
        {
            if (itr->second == nullptr)
            {
                buffer->SkipRead(header.size);
                continue;
            }

            if (!itr->second(buffer, header, wmoRoot, wmoObject))
            {
                assert(false);
            }
        }

    } while (buffer->readData != buffer->size);

    return true;
}
