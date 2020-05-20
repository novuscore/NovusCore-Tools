#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

#pragma pack(push, 1)
// Old Water information Chunk
struct MCLQ
{
    vec2 height; // min-max
    struct SLVert
    {
        union 
        {
            struct SWVert
            {
                i8 depth;
                i8 flow0Pct;
                i8 flow1Pct;
                i8 filler;
                f32 height;
            } waterVert;
            struct SOVert
            {
                i8 depth;
                i8 foam;
                i8 wet;
                i8 filler;
            } oceanVert;
            struct SMVert
            {
                u16 s;
                u16 t;
                f32 height;
            } magmaVert;
        };
    } verts[9*9];

    struct SLTiles 
    {
        i8 tiles[8][8];
        // 0x0f or 0x8 mean don't render (?, TC: 0xF)
        // &0xf: liquid type (1: ocean, 3: slime, 4: river, 6: magma)
        // 0x10:
        // 0x20:
        // 0x40: not low depth (forced swimming ?)
        // 0x80: fatigue (?, TC: yes)
    } tiles;

    u32 nFlowvs;
    struct SWFlowv
    {
        vec3 spherePosition;
        f32 sphereRadius;
        f32 velocity;
        f32 amplitude;
        f32 frequency;
    } flowvs[2]; // always 2 in file, independent on nFlowvs.
};
#pragma pack(pop)