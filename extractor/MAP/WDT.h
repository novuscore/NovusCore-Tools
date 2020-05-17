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
#include <Utils/ByteBuffer.h>
#include "ADTStructs.h"
#include <vector>

#pragma pack(push, 1)
enum MPHD_Flags 
{
    wdt_uses_global_map_obj = 0x0001,                   // Use global map object definition.
    adt_has_mccv = 0x0002,                              // >= Wrath adds color: ADT.MCNK.MCCV. with this flag every ADT in the map _must_ have MCCV chunk at least with default values, else only base texture layer is rendered on such ADTs.
    adt_has_big_alpha = 0x0004,                         // shader = 2. Decides whether to use _env terrain shaders or not: funky and if MCAL has 4096 instead of 2048(?)
    adt_has_doodadrefs_sorted_by_size_cat = 0x0008,     // if enabled, the ADT's MCRF(m2 only)/MCRD chunks need to be sorted by size category
    FLAG_LIGHTINGVERTICES = 0x0010,                     // >= Cata adds second color: ADT.MCNK.MCLV -- This appears to be deprecated and forbidden in 8.x?
    adt_has_upside_down_ground = 0x0020,                // >= Cata Flips the ground display upside down to create a ceiling
    unk_0x0040 = 0x0040,                                // >= Mists ??? -- Only found on Firelands2.wdt (but only since MoP) before Legion
    adt_has_height_texturing = 0x0080,                  // >= Mists shader = 6. Decides whether to influence alpha maps by _h+MTXP: (without with)
                                                        // also changes MCAL size to 4096 for uncompressed entries
    unk_0x0100 = 0x0100,                                // >= Legion implicitly sets 0x8000
    wdt_has_maid = 0x0200,                              // >= Battle (8.1.0.28294) client will load ADT using FileDataID instead of filename formatted with "%s\\%s_%d_%d.adt"
    unk_0x0400 = 0x0400,
    unk_0x0800 = 0x0800,
    unk_0x1000 = 0x1000,
    unk_0x2000 = 0x2000,
    unk_0x4000 = 0x4000,
    unk_0x8000 = 0x8000,                                // >= Legion implicitly set for map ids 0, 1, 571, 870, 1116 (continents). Affects the rendering of _lod.adt

    mask_vertex_buffer_format = adt_has_mccv | FLAG_LIGHTINGVERTICES,                    // CMap::LoadWdt
    mask_render_chunk_something = adt_has_height_texturing | adt_has_big_alpha,   // CMapArea::PrepareRenderChunk, CMapChunk::ProcessIffChunks
};

struct MPHD
{
    u32 token;
    u32 size;

    u32 flags;
    u32 something;
    u32 unused[6];
};

constexpr u32 NUM_SM_AREA_INFO = 64 * 64;

struct MAIN
{
    u32 token;
    u32 size;

    struct SMAreaInfo
    {
        u32 Flag_HasADT : 1;
        u32 Flag_Loaded : 1; // Unused
        u32 asyncId; // Only set during runtime, so we don't use it
    };
    SMAreaInfo mapAreaInfo[NUM_SM_AREA_INFO];
};

enum AlphaType
{
    SmallAlpha,
    BigAlpha
};

// TODO: for WMO only maps, load MWMO, MODF chunks
class WDT
{
public:
    WDT(std::shared_ptr<ByteBuffer> file, std::string fileName, std::string filePath);
    std::vector<u32> Convert();

    AlphaType GetAlphaType();

private:
    std::shared_ptr<ByteBuffer> _file;
    std::string _fileName;
    std::string _filePath;

    AlphaType _alphaType;
};
#pragma pack(pop)