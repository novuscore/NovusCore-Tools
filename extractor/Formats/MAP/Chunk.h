/*
# MIT License
# Copyright(c) 2018-2020 NovusCore
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/
#pragma once
#include <NovusTypes.h>
#include "Cell.h"
#include "../FileChunk/Chunks/FileChunkHeader.h"

#include <vector>

constexpr i32 MAP_CHUNK_TOKEN = 1128812107;  // UTF8 -> Binary -> Decimal for "chnk"
constexpr i32 MAP_CHUNK_VERSION = 1;

constexpr u32 MAP_CHUNKS_PER_MAP_SIDE = 64;
constexpr u16 MAP_CHUNK_INVALID = std::numeric_limits<u16>().max();

constexpr u16 MAP_CELLS_PER_CHUNK_SIDE = 16;
constexpr u16 MAP_CELLS_PER_CHUNK = MAP_CELLS_PER_CHUNK_SIDE * MAP_CELLS_PER_CHUNK_SIDE;

constexpr u32  MAP_CHUNK_ALPHAMAP_NUM_CHANNELS = 4;
constexpr u32 MAP_CHUNK_ALPHAMAP_BYTE_SIZE = CELL_ALPHAMAP_SIZE * MAP_CELLS_PER_CHUNK * MAP_CHUNK_ALPHAMAP_NUM_CHANNELS;

#pragma pack(push, 1)
struct HeightHeader
{
    u8 hasHeightBox = false;
    f32 gridMinHeight = 20000;
    f32 gridMaxHeight = -20000;
};

struct HeightPlane
{
    // For future implementation: https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/351404-traceline-intersection-collision-detection-height-limit.html
    i16 heightPoints[3 * 3] = { 0 };
};

struct HeightBox
{
    HeightPlane minHeight;
    HeightPlane maxHeight;
};

struct AlphaMap
{
    u8 alphaMap[4096] = { 0 }; // 4096 pixels per alpha map
};

struct MapObjectPlacement
{
    u32 nameID;
    vec3 position;
    vec3 rotation;
    u16 scale;
};

struct CellLiquidHeader
{
    // Packed Format
    // Bit 1-7 (numInstances)
    // Bit 8 (hasAttributes)
    u8 packedData = 0;

    u8 cellID = 0;
};

struct CellLiquidInstance
{
    u8 liquidType = 0;

    // Packed Format
    // Bit 1-6 (liquidVertexFormat)
    // Bit 7 (hasBitMaskForPatches)
    // Bit 8 (hasVertexData)
    u8 packedData = 0;

    hvec2 heightLevel = hvec2(0.f, 0.f); // Min, Max
    u8 packedOffset = 0; // X, Y
    u8 packedSize = 0; // Width, Height
};


// The following 4 Structs only exists for the purpose of being able to sizeof() inside for Mh2o::Read
// This makes it easier to read the code (The actual) structs in memory are arrays one after another
struct LiquidVertexFormat_Height_Depth
{
    f32 heightMap;
    u8 depthMap;
};

struct LiquidVertexFormat_Height_UV
{
    f32 heightMap;

    u16 uvX;
    u16 uvY;
};

struct LiquidVertexFormat_Depth
{
    u8 depthMap;
};

struct LiquidVertexFormat_Height_UV_Depth
{
    f32 heightMap;

    u16 uvX;
    u16 uvY;

    u8 depthMap;
};

struct MapChunkHeader
{
    u32 token = MAP_CHUNK_TOKEN;
    u32 version = 4;
};

struct Chunk
{
    MapChunkHeader header;

    HeightHeader heightHeader;
    HeightBox heightBox;

    std::vector<MapObjectPlacement> mapObjectPlacements;

    Cell cells[MAP_CELLS_PER_CHUNK];
    u8 alphaMapData[MAP_CHUNK_ALPHAMAP_BYTE_SIZE];
};
#pragma pack(pop)