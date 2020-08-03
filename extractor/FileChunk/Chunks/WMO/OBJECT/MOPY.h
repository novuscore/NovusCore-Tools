#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MOPYFLags
{
    Unk_0x1 = 1 << 0,
    NoCamCollide = 1 << 1,
    Detail = 1 << 2,
    Collision = 1 << 3, // Turns off water ripple effects, and is used for ghost material triangles)
    Hint = 1 << 4,
    Render = 1 << 5,
    Unk_0x40 = 1 << 6,
    CollideHit = 1 << 7,
};

struct WMO_ROOT;
struct WMO_OBJECT;
struct ChunkHeader;
struct MOPY
{
    struct
    {
        u8 Unk_0x1 : 1;
        u8 NoCamCollide : 1;
        u8 Detail : 1;
        u8 Collision : 1; // Turns off water ripple effects, and is used for ghost material triangles)
        u8 Hint : 1;
        u8 Render : 1;
        u8 Unk_0x40 : 1;
        u8 CollideHit : 1;

        // 0xFF is used for triangles that can only collide, meaning they are not rendered.

        bool IsTransFace() { return Unk_0x1 && (Detail || Render); }
        bool IsColor() { return !Collision; }
        bool IsRenderFace() { return Render && !Detail; }
        bool IsCollidable() { return Collision || IsRenderFace(); }
    } flags;

    u8 materialId; // This is an index into MOMT

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};