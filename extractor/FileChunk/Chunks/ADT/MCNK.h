#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

enum class MCNKFlags
{
    HasMCSH             = 1 << 0,
    Impass              = 1 << 1,
    LiquidRiver         = 1 << 2,
    LiquidOcean         = 1 << 3,
    LiquidMagma         = 1 << 4,
    LiquidSlime         = 1 << 5,
    HasMMCV             = 1 << 6,
    Unk_0x80            = 1 << 7,
    // Next 7 bits unused
    DoNotFixAlphaMap    = 1 << 15,
    HighResHoles        = 1 << 16,
    // Last 15 bits unused
};

#pragma pack(push, 1)
// Cell Information Chunk
struct WDT;
struct ADT;
struct ChunkHeader;
struct MCNK
{
    u32 flags = 0;
    u32 posX = 0;
    u32 posY = 0;
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
    vec3 position = vec3(0,0,0); // Location is stored as Z, X, Y
    u32 offsetMCCV = 0; // Vertex Colors
    u32 offsetMCLV = 0; // Cata+
    u32 pad2 = 0;

    static bool Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt);
};
#pragma pack(pop)
