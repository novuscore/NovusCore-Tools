#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Sound Emitter information Chunk
struct MCSE
{
    u32 entryId = 0; // Foreign ID
    vec3 position = vec3(0,0,0);
    vec3 size = vec3(0,0,0); // Apparently seems unused
};
#pragma pack(pop)