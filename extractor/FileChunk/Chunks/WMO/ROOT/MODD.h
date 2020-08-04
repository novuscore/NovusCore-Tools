#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

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
    struct MODDData
    {
        u32 packedValue = 0; // Contains (Name Index into MODN and Flags)
        vec3 position;
        vec4 rotation; // Quaternion
        f32 scale = 0;
        u32 color = 0; // This is (BGRA) (Overrides pc_sunColor when A is != 0xff, A is a MOLT index and that's used instead the RGB given here, taking distance and intensity into account)
    
        u32 GetNameIndex() { return packedValue & 0xFFFFFF; }
        u8 GetFlags() { return (packedValue >> 24) & 0xFF; }
    };

    std::vector<MODDData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const ChunkHeader& header, WMO_ROOT& wmo);
};
#pragma pack(pop)