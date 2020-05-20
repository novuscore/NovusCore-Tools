#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MPHDFlags
{
    UsesGlobalMapObj                    = 0x0001,   // Use global map object definition.
    AdtHasMCCV                          = 0x0002,   // >= Wrath adds color: ADT.MCNK.MCCV. with this flag every ADT in the map _must_ have MCCV chunk at least with default values, else only base texture layer is rendered on such ADTs.
    AdtHasBigAlpha                      = 0x0004,   // shader = 2. Decides whether to use _env terrain shaders or not: funky and if MCAL has 4096 instead of 2048(?)
    AdtHasDoodadrefsSortedBySizeCat     = 0x0008,   // if enabled, the ADT's MCRF(m2 only)/MCRD chunks need to be sorted by size category
    FlagLightingVertices                = 0x0010,   // >= Cata adds second color: ADT.MCNK.MCLV -- This appears to be deprecated and forbidden in 8.x?
    AdtHasUpsideDownGround              = 0x0020,   // >= Cata Flips the ground display upside down to create a ceiling
    Unk_0x40                            = 0x0040,   // >= Mists ??? -- Only found on Firelands2.wdt (but only since MoP) before Legion
    AdtHasHeightTexturing               = 0x0080,   // >= Mists shader = 6. Decides whether to influence alpha maps by _h+MTXP: (without with)
                                                    // also changes MCAL size to 4096 for uncompressed entries
    Unk_0x100                           = 0x0100,   // >= Legion implicitly sets 0x8000
    UdtHasMaid                          = 0x0200,   // >= Battle (8.1.0.28294) client will load ADT using FileDataID instead of filename formatted with "%s\\%s_%d_%d.adt"
    Unk_0x400                           = 0x0400,
    Unk_0x800                           = 0x0800,
    Unk_0x1000                          = 0x1000,
    Unk_0x2000                          = 0x2000,
    Unk_0x4000                          = 0x4000,
    Unk_0x8000                          = 0x8000,   // >= Legion implicitly set for map ids 0, 1, 571, 870, 1116 (continents). Affects the rendering of _lod.adt
    
    MaskVertexBufferFormat = AdtHasMCCV | FlagLightingVertices, // CMap::LoadWdt
    MaskRenderChunkSomething = AdtHasHeightTexturing | AdtHasBigAlpha,  // CMapArea::PrepareRenderChunk, CMapChunk::ProcessIffChunks
};

#pragma pack(push, 1)
// Cell Information Chunk
struct WDT;
struct ChunkHeader;
struct MPHD
{
    u32 flags;
    u32 something;
    u32 pad1[6];

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, WDT& wdt);
};
#pragma pack(pop)
