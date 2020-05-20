#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Height Points Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MFBO
{
    struct HeightPlane
    {
        // For future implementation: https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/351404-traceline-intersection-collision-detection-height-limit.html
        i16 heightPoints[3 * 3] = { 0,0,0,0,0,0,0 };
    };

    HeightPlane max = HeightPlane();
    HeightPlane min = HeightPlane();

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
