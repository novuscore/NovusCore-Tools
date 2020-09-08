#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct WMO_ROOT;
struct WMO_OBJECT;
struct FileChunkHeader;
struct MOGP
{
    u32 groupName; // This is an index into MOGN
    u32 descriptiveGroupName; // This is an index into MOGN
    u32 flags; // WMO Object Flags

    vec3 boundingBoxMin;
    vec3 boundingBoxMax;

    u16 portalStart; // This is an index into MOPR
    u16 portalCount;

    u16 transBatchCount;
    u16 intBatchCount;
    u16 extBatchCount;
    u16 pad; // Might be data

    u8 fogIds[4]; // These are indexes into MFOG
    u32 groupLiquid; // What does this mean?

    u32 uniqueId; // This is a foreign key into WMOAreaTableRec::m_WMOGroupId

    u32 flags2; // Used in MOP+
    u32 unk;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};