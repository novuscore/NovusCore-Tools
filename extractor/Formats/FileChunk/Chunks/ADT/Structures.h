#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include "../../../MAP/Chunk.h"

#include <vector>
#include <memory>

namespace Wdt
{
    struct Wdt;
}
struct FileChunkHeader;

#pragma pack(push, 1)
namespace Adt
{
    struct Adt;

    struct MhdrFlags
    {
        u32 MFBO : 1;
        u32 NORTHREND : 1;
    };

    // Map Header Chunk
    struct Mhdr
    {
        MhdrFlags flags;

        u32 mcin = 0;
        u32 mtex = 0;
        u32 mmdx = 0;
        u32 mmid = 0;
        u32 mwmo = 0;
        u32 mwid = 0;
        u32 mddf = 0;
        u32 modf = 0;
        u32 mfbo = 0;
        u32 mh2o = 0;
        u32 mtxf = 0;
        u8 mamp_value = 0;
        u8 padding[3] = { 0 };
        u32 unused[3] = { 0 };

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    // Map Cell Mapping Chunk
    struct Mcin
    {
        struct McinData
        {
            u32 offset = 0; // Absolute
            u32 size = 0;
            u32 flags = 0; // Always 0. Only set in the client (FLAG_LOADED = 1)

            u8 padding[4] = { 0 }; // This is used inside the client (asyncId)
        };

        std::vector<McinData> data;

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    // Texture List Chunk
    struct Mtex
    {
        u32 size = 0;
        u8* filenames = nullptr; // Can store multiple names

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    // M2 Chunk (Describes which M2s are used in this map chunk. These offsets are referenced in MMID)
    struct Mmdx
    {
        u32 size = 0;
        u8* filenames = nullptr; // Can store multiple names

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };
    
    // M2 Chunk (Describes an offset into MMDX. These offsets are referenced in MDDF)
    struct Mmid
    {
        u32 size;
        u8* offsets; // Array of u32s

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    // WMO Chunk (Describes which WMOs are used in this map chunk. These offsets are referenced in MWID)
    struct Mwmo
    {
        u32 size = 0;
        u8* filenames = nullptr; // Can store multiple names

        static bool ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt::Wdt& wdt);
        static bool ReadADT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    // WMO Chunk (Describes an offset into MWID. These offsets are referenced in MODF)
    struct Mwid
    {
        u32 size = 0;
        u8* offset = nullptr; // Array of u32s

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    struct MddfFlags
    {
        u16 Biodome : 1;
        u16 Shrubbery : 1;
        u16 Unk_0x4 : 1;
        u16 Unk_0x8 : 1;
        u16 : 1;
        u16 LiquidKnown : 1;
        u16 EntryIsFileDataId : 1;
        u16 : 1;
        u16 Unk_0x100 : 1;
    };

    // Placement information for doodads Chunk
    struct Mddf
    {
        struct MddfData
        {
            u32 nameId = 0;
            u32 uniqueId = 0;
            vec3 position = vec3(0, 0, 0);
            vec3 rotation = vec3(0, 0, 0);
            u16 scale = 0;
            MddfFlags flags;
        };

        std::vector<MddfData> data;

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    struct ModfFlags
    {
        u16 Destroyable : 1;
        u16 UseLod : 1;
        u16 HasScale : 1;
        u16 EntryIsFiledataId : 1;
    };

    // Placement information for WMOs Chunk
    struct Modf
    {
        struct ModfData
        {
            u32 nameId = 0; // Reference to MWID chunk
            u32 uniqueId = 0; // This ID should be unique
            vec3 position = vec3(0, 0, 0);
            vec3 rotation = vec3(0, 0, 0);

            // Bounding Box
            vec3 min = vec3(0, 0, 0);
            vec3 max = vec3(0, 0, 0);

            u16 flags = 0;
            u16 doodadSet = 0;
            u16 nameSet = 0;
            ModfFlags scale;
        };

        std::vector<ModfData> data;

        static bool ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt::Wdt& wdt);
        static bool ReadADT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    constexpr u32 MAX_VERTICES_PER_LIQUID_INSTANCE = 81;

    // Liquid Information Chunk
    struct Mh2o
    {
        struct LiquidInstance
        {
            u16 liquidType = 0; // Foreign Key (Referencing LiquidType.dbc)
            u16 liquidVertexFormat = 0; // Classic, TBC and WOTLK Only (Cata+ Foreign Key)

            f32 minHeightLevel = 0;
            f32 maxHeightLevel = 0;

            u8 xOffset = 0;
            u8 yOffset = 0;
            u8 width = 0;
            u8 height = 0;

            u32 bitmapExistsOffset = 0; // Will contain bytes equals to height (For every "Row" we have cells on, we need at least 8 bits to display which is 1 byte)
            u32 vertexDataOffset = 0; // Vertex Data, can be in 2 formats for WOTLK and will always contain vertexCount entries for both arrays (f32* heightMap, char* depthMap), (f32* heightMap, UVEntry* uvMap)
        };

        struct LiquidAttributes
        {
            u64 fishable = 0; // seems to be usable as visibility information.
            u64 deep = 0; // Might be related to fatigue area if bit set.

            // Note that these are bitmasks.
        };

        struct Mh2oData
        {
            u32 instancesOffset = 0;
            u32 layerCount = 0;
            u32 attributesOffset = 0;
        };

        std::vector<Mh2oData> data;
        std::vector<CellLiquidHeader> headers;
        std::vector<CellLiquidInstance> instances;
        std::vector<LiquidAttributes> attributes;

        std::vector<u8> bitMaskForPatchesData;
        std::vector<u8> vertexData;

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    struct McnkFlags
    {
        u32 HasMCSH : 1;
        u32 Impass : 1;
        u32 LiquidRiver : 1;
        u32 LiquidOcean : 1;
        u32 LiquidMagma : 1;
        u32 LiquidSlime : 1;
        u32 HasMMCV : 1;
        u32 Unk_0x80 : 1;
        u32 : 7;
        u32 DoNotFixAlphaMap : 1;
        u32 HighResHoles : 1;
    };

    // Cell Information Chunk
    struct Mcnk
    {
        u32 token = 0;
        u32 size = 0;

        McnkFlags flags;
        u32 indexX = 0;
        u32 indexY = 0;
        u32 numLayers = 0;
        u32 numDoodadRefs = 0;

        union
        {
            u64 holesHighRes = 0; // This is used after patch 5.3 ish

            struct
            {
                u32 offsetMCVT; // Height
                u32 offsetMCNR; // Normal
            };
        };

        u32 offsetMCLY = 0; // Layer
        u32 offsetMCRF = 0; // Doodads & WMO Refs (We should read this as 2x u32 arrays using numDoodadRefs and NumMapObjectRefs)
        u32 offsetMCAL = 0; // Alpha
        u32 sizeMCAL = 0; // Size Alpha
        u32 offsetMCSH = 0; // Shadow
        u32 sizeMCSH = 0; // Size Shadow
        u32 areaId = 0;
        u32 numMapObjectRefs = 0; // Number of WMO Refs
        u16 holesLowRes = 0;
        u16 pad1 = 0;
        u8 lowQualityTextureMap[16] = { 0 }; // Used to determine where doodads are shown. Values are an array of two bit
        u8 disableDoodadMap[8] = { 0 }; // Used to determine if Doodads are disabled (This takes priority over lowQualityTextureMap) Values are an array of 1 bit
        u32 offsetMCSE = 0; // Sound Emitters
        u32 numSoundEmitters = 0;
        u32 offsetMCLQ = 0; // Liquid (Another system, used mainly in classic, TBC and used just for TBC in WOTLK)
        u32 sizeMCLQ = 0;
        vec3 position = vec3(0, 0, 0); // Location is stored as Z, X, Y
        u32 offsetMCCV = 0; // Vertex Colors
        u32 offsetMCLV = 0; // Cata+
        u32 pad2 = 0;

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    }; 
    
    // Chunk Height Data
    struct Mcvt
    {
        f32 height[9 * 9 + 8 * 8] = { 0.0f };
    };

    // Vertex shading information Chunk
    struct Mccv
    {
        struct MCCVEntry
        {
            u8 blue = 127;
            u8 green = 127;
            u8 red = 127;
            u8 alpha = 127;
        } entries[9 * 9 + 8 * 8];
    };

    struct Mcnr
    {
        struct MCNREntry
        {
            i8 normal[3];
        } entries[9 * 9 + 8 * 8];
    };

    struct MclyFlags
    {
        u32 AnimationRotation_X : 1;
        u32 AnimationRotation_Y : 1;
        u32 AnimationRotation_Z : 1;
        u32 AnimationSpeed_X : 1;
        u32 AnimationSpeed_Y : 1;
        u32 AnimationSpeed_Z : 1;
        u32 AnimationEnabled : 1;
        u32 Overbright : 1;
        u32 UseAlphaMap : 1;
        u32 CompressedAlphaMap : 1;
        u32 UseCubeMapReflection : 1;
        u32 Unk_0x800 : 1;
        u32 Unk_0x1000 : 1;
    };

    // Texture Chunk (Describes the texture layers for this chunk)
    struct Mcly
    {
        struct MclyData
        {
            u32 textureId = 0;
            MclyFlags flags;
            u32 offsetInMCAL = 0;
            u32 effectId = 0;
        };

        std::vector<MclyData> data;
    };

    struct Mcrf
    {
        std::vector<u32> doodadRefs;
        std::vector<u32> mapObjectRefs;
    };

    // Shadow Map (Describes the shadowmap for this chunk)
    struct Mcsh
    {
        // These are actually 64x64 1 bit values, each bit can be on or off "black/white", the order is LSB first.
        u8 bitMask[8][8];
    };

    struct Mcal
    {
        struct McalData
        {
            u8 alphaMap[4096];
        };

        std::vector<McalData> data;
    };

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
        } verts[9 * 9];

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

    // Sound Emitter information Chunk
    struct Mcse
    {
        struct McseData
        {
            u32 entryId = 0; // Foreign ID
            vec3 position = vec3(0, 0, 0);
            vec3 size = vec3(0, 0, 0); // Apparently seems unused
        };

        std::vector<McseData> data;
    };
    
    // Height Points Chunk
    struct Mfbo
    {
        struct HeightPlane
        {
            // For future implementation: https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/351404-traceline-intersection-collision-detection-height-limit.html
            i16 heightPoints[3 * 3] = { 0,0,0,0,0,0,0 };
        };

        HeightPlane max = HeightPlane();
        HeightPlane min = HeightPlane();

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };

    struct MtxfFlags
    {
        u32 UseCubemap : 1;
        u32 : 3;
        u32 TextureScalingLayer0 : 1;
        u32 TextureScalingLayer1 : 1;
        u32 TextureScalingLayer2 : 1;
        u32 TextureScalingLayer3 : 1;
    };

    // Texture Effects Chunk
    struct Mtxf
    {
        struct MtxfData
        {
            MtxfFlags flags;
        };

        std::vector<MtxfData> data;

        static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt);
    };
}
#pragma pack(pop)