#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct ChunkHeader;
struct MCVP
{
    struct ConvexVolumePlane
    {
        vec3 normal;
        f32 distance = 0;
    };
    std::vector<ConvexVolumePlane> convexVolumePlanes;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};