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

#define ADT_CELLS_PER_GRID 16
#define ADT_CELL_SIZE 8
#define ADT_GRID_SIZE (ADT_CELLS_PER_GRID * ADT_CELL_SIZE)

// ADT_CELL_SIZE (THIS REPRESENTS THE AMOUNT OF CELLS INSIDE A CHUNK; NOT THE SIZE OF A CELL) TY TC

#define NOVUSMAP_MVER_TOKEN 1297499474
#define NOVUSMAP_MHDR_TOKEN 1296581714
#define NOVUSMAP_MCIN_TOKEN 1296255310
#define NOVUSMAP_MCNK_TOKEN 1296256587
#define NOVUSMAP_MFBO_TOKEN 1296450127 
#define NOVUSMAP_MH2O_TOKEN 1296577103
#define NOVUSMAP_MCVT_TOKEN 1296258644

enum LIQUID_ID
{
    LIQUID_ID_WATER = 0,
    LIQUID_ID_OCEAN = 1,
    LIQUID_ID_MAGMA = 2,
    LIQUID_ID_SLIME = 3
};
enum LIQUID_TYPE
{
    LIQUID_TYPE_NO_WATER = 0x00,
    LIQUID_TYPE_WATER = 0x01,
    LIQUID_TYPE_OCEAN = 0x02,
    LIQUID_TYPE_MAGMA = 0x04,
    LIQUID_TYPE_SLIME = 0x08,
    LIQUID_TYPE_DARK_WATER = 0x10
};

#define MAP_LIQUID_NO_TYPE 0x0001
#define MAP_LIQUID_NO_HEIGHT 0x0002

#pragma pack(push, 1)
struct MVER
{
    MVER() : token(0), size(0), version(0) {}

    u32 token;
    u32 size;
    u32 version;
};

struct MCVT
{
    MCVT() : token(0), size(0) {}

    u32 token;
    u32 size;
};

struct MCLQ
{
    MCLQ() : liquid(), flags(), data() {}

    u32 token = 0;
    u32 size = 0;

    f32 heightMin = 0;
    f32 heightMax = 0;

    struct LIQUID
    {
        u32 light;
        f32 height;
    } liquid[ADT_CELL_SIZE + 1][ADT_CELL_SIZE + 1];

    // 1<<0 - ocean
    // 1<<1 - lava/slime
    // 1<<2 - water
    // 1<<6 - all water
    // 1<<7 - dark water
    // == 0x0F - not show liquid
    u8 flags[ADT_CELL_SIZE][ADT_CELL_SIZE];
    u8 data[84];
};

struct MCNK
{
    MCNK() : token(0), size(0), flags(0), ix(0), iy(0), layers(0), doodadRefs(0), offsetMcvt(0), offsetMcnr(0), offsetMcly(0),
             offsetMcrf(0), offsetMcal(0), sizeMcal(0), offsetMcsh(0), sizeMcsh(0), areaId(0), mapObjectReferences(0), holes(0),
             lowQualityTextureMap(), predTex(0), effectDoodad(0), offsetMcse(0), soundEmitters(0), offsetMclq(0), sizeMclq(0), zPos(0), xPos(0), yPos(0),
             offsetMccv(0), props(0), effectId(0) {}

    u32 token;
    u32 size;
    u32 flags;
    u32 ix;
    u32 iy;
    u32 layers;
    u32 doodadRefs;
    u32 offsetMcvt;
    u32 offsetMcnr;
    u32 offsetMcly;
    u32 offsetMcrf;
    u32 offsetMcal;
    u32 sizeMcal;
    u32 offsetMcsh;
    u32 sizeMcsh;
    u32 areaId;
    u32 mapObjectReferences;
    u32 holes;
    u8 lowQualityTextureMap[16];
    u32 predTex;
    u32 effectDoodad;
    u32 offsetMcse;
    u32 soundEmitters;
    u32 offsetMclq;
    u32 sizeMclq;
    f32 zPos;
    f32 xPos;
    f32 yPos;
    u32 offsetMccv;
    u32 props;
    u32 effectId;
};

struct MCIN
{
    MCIN() : token(0), size(0) {}

    u32 token;
    u32 size;
};

struct MFBO
{
    MFBO() : token(0), size(0), max(), min() {}

    struct HeightPlane
    {
        // For future implementation: https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/351404-traceline-intersection-collision-detection-height-limit.html
        i16 heightPoints[3 * 3] = { 0,0,0,0,0,0,0 };
    };

    u32 token;
    u32 size;

    HeightPlane max;
    HeightPlane min;
};

struct MH2O
{
    MH2O() : token(0), size(0), liquidHeaders() {}

    u32 token;
    u32 size;

    struct LiquidHeader
    {
        u32 offsetInformation;
        u32 layers;
        u32 offsetRenderMask;
    } liquidHeaders[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];
};

struct MHDR
{
    MHDR() : token(0), size(0), flags(0), offsetMcin(0), offsetMtex(0), offsetMmdx(0),
             offsetMmid(0), offsetMwmo(0), offsetMddf(0), offsetModf(0), offsetMfbo(0),
             offsetMh2o(0), offsetMtfx(0), pad4(0), pad5(0), pad6(0), pad7(0) {}

    enum MHDRFlags
    {
        mhdr_MFBO = 1,      // contains a MFBO chunk.
        mhdr_northrend = 2, // is set for some northrend ones.
    };

    u32 token;
    u32 size;

    u32 flags;      // &1: MFBO, &2: unknown. in some Northrend ones.
    u32 offsetMcin; //Positions of MCNK's
    u32 offsetMtex; //List of all the textures used
    u32 offsetMmdx; //List of all the md2's used
    u32 offsetMmid; //Offsets into MMDX list for what each ID is
    u32 offsetMwmo; //list of all the WMO's used
    u32 offsetMwid; //Offsets into MWMO list for what each ID is
    u32 offsetMddf; //Doodad Information
    u32 offsetModf; //WMO Positioning Information
    u32 offsetMfbo; // tbc, wotlk; only when flags&1
    u32 offsetMh2o; // wotlk
    u32 offsetMtfx; // wotlk
    u32 pad4;
    u32 pad5;
    u32 pad6;
    u32 pad7;
};
#pragma pack(pop)