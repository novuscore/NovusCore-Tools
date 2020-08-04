#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

enum class MFOGFlags
{
    InfiniteRaidus = 1 << 0,
    Unk_0x10 = 1 << 4
};

struct WMO_ROOT;
struct ChunkHeader;
struct MFOG
{
    struct MFOGData
    {
        u32 flags = 0;
        vec3 position;
        f32 smallRadius = 0;
        f32 largeRadius = 0;

        // Fog
        f32 fogEnd = 0;
        f32 fogStartScalar = 0;
        u32 fogColor = 0; // This is (BGRA)

        // Underwater Fog
        f32 uwfogEnd = 0;
        f32 uwfogStartScalar = 0;
        u32 uwfogColor = 0; // This is (BGRA)
    };

    std::vector<MFOGData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};