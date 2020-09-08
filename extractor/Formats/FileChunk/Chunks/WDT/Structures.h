#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include "../../../MAP/Chunk.h"

#include <vector>
#include <memory>

struct FileChunkHeader;

constexpr u32 NUM_SM_AREA_INFO = 64 * 64;

#pragma pack(push, 1)
namespace Wdt
{
    struct Wdt;

    // Cell Information Chunk
    struct Main
    {
        struct SMAreaInfo
        {
            u32 hasADT = 0;
            u32 loaded = 0; // Unused
        };
        SMAreaInfo MapAreaInfo[NUM_SM_AREA_INFO];

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt& wdt);
    };

    struct MphdFlags
    {
        u32 UsesGlobalMapObj : 1;
        u32 AdtHasMCCV : 1;
        u32 AdtHasBigAlpha : 1;
        u32 AdtHasDoodadrefsSortedBySizeCat : 1;
        u32 FlagLightingVertices : 1;
        u32 AdtHasUpsideDownGround : 1;
        u32 Unk_0x40 : 1;
        u32 AdtHasHeightTexturing : 1;
        u32 Unk_0x100 : 1;
        u32 UdtHasMaid : 1;
        u32 Unk_0x400 : 1;
        u32 Unk_0x800 : 1;
        u32 Unk_0x1000 : 1;
        u32 Unk_0x2000 : 1;
        u32 Unk_0x4000 : 1;
        u32 Unk_0x8000 : 1;
    };

    // Cell Information Chunk
    struct Mphd
    {
        MphdFlags flags;
        u32 something;
        u32 pad1[6];

        bool MaskVertexBufferFormat() { return flags.AdtHasMCCV | flags.FlagLightingVertices; }
        bool MaskRenderChunkSomething() { return flags.AdtHasHeightTexturing | flags.AdtHasBigAlpha; }

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt& wdt);
    };
}
#pragma pack(pop)