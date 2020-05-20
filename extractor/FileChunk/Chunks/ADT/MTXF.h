#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MTXFFlags
{
    UseCubemap = 1 << 0,            // Disables loading of specular and height maps
    TextureScalingLayer0 = 1 << 4,  // (MOP+) Texture Scaling
    TextureScalingLayer1 = 1 << 5,  // (MOP+) Texture Scaling
    TextureScalingLayer2 = 1 << 6,  // (MOP+) Texture Scaling
    TextureScalingLayer3 = 1 << 7   // (MOP+) Texture Scaling
};

#pragma pack(push, 1)
// Texture Effects Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MTXF
{
    u32 flags = 0;

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
