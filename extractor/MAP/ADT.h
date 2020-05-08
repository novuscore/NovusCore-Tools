/*
# MIT License

# Copyright(c) 2018-2019 NovusCore

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
#include <Utils/ByteBuffer.h>
#include "ADTStructs.h"
#include "../MPQ/MPQFile.h"
#include "Cell.h"

constexpr i32 MAP_CHUNK_TOKEN = 1313685840;
constexpr i32 MAP_CHUNK_VERSION = 808464434;

constexpr u32 MAP_CHUNKS_PER_MAP_SIDE = 64;
constexpr u16 MAP_CHUNK_INVALID = std::numeric_limits<u16>().max();

constexpr u16 MAP_CELLS_PER_CHUNK_SIDE = 16;
constexpr u16 MAP_CELLS_PER_CHUNK = MAP_CELLS_PER_CHUNK_SIDE * MAP_CELLS_PER_CHUNK_SIDE;

#pragma pack(push, 1)
struct ChunkHeader
{
    u32 token = MAP_CHUNK_TOKEN;
    u32 version = MAP_CHUNK_VERSION;
};

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

struct Chunk
{
    ChunkHeader chunkHeader;

    HeightHeader heightHeader;
    HeightBox heightBox;

    Cell cells[MAP_CELLS_PER_CHUNK];
};

class ADT
{
public:
    ADT(std::shared_ptr<MPQFile> file, std::string fileName, std::string filePath);
    void Convert();
    u8 GetLiquidIdFromType(u16 type);

private:
    

    // Grid Data Storage
    //u16 areaIds[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];
    //f32 heightMap[ADT_CELLS_PER_GRID * ADT_CELLS_PER_GRID][(ADT_CELL_SIZE + 1) * (ADT_CELL_SIZE + 1) + ADT_CELL_SIZE * ADT_CELL_SIZE];

    //i16 heightBoxMax[3][3];
    //i16 heightBoxMin[3][3];

    std::shared_ptr<MPQFile> _file;
    std::string _fileName;
    std::string _filePath;
};
#pragma pack(pop)