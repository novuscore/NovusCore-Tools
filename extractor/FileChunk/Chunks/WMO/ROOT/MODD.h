#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MODDFlags
{
    AcceptProjectedTexture = 1 << 0,
    Unk_0x2 = 1 << 1,
    Unk_0x4 = 1 << 2,
    Unk_0x8 = 1 << 3
};

struct WMO_ROOT;
struct ChunkHeader;
#pragma pack(push, 1)
struct MODD
{
    u32 nameIndex = 0; // Index into MODN
    u8 flags = 0;
    vec3 position;
    vec4 rotation; // Quaternion
    f32 scale = 0;
    u32 color = 0; // This is (BGRA) (Overrides pc_sunColor when A is != 0xff, A is a MOLT index and that's used instead the RGB given here, taking distance and intensity into account)

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};
#pragma pack(pop)