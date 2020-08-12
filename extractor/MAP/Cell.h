/*
    MIT License

    Copyright (c) 2018-2020 NovusCore

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once
#include <NovusTypes.h>

constexpr u16 CELL_OUTER_GRID_SIDE = 9;
constexpr u16 CELL_OUTER_GRID_SIZE = CELL_OUTER_GRID_SIDE * CELL_OUTER_GRID_SIDE;

constexpr u16 CELL_INNER_GRID_SIDE = 8;
constexpr u16 CELL_INNER_GRID_SIZE = CELL_INNER_GRID_SIDE * CELL_INNER_GRID_SIDE;

constexpr u16 CELL_TOTAL_GRID_SIZE = CELL_OUTER_GRID_SIZE + CELL_INNER_GRID_SIZE;

constexpr u32 CELL_ALPHAMAP_SIZE = 64 * 64;

#pragma pack(push, 1)
struct LiquidData
{
    u8 hasMultipleLiquidTypes = false;
    u8 offsetX = 255;
    u8 offsetY = 255;
    u8 width = 0;
    u8 height = 0;
    u8 liquidFlags = 0;
    u16 liquidEntry = 0;
    f32 level = 20000;
    u32 layers = 0;
    f32 liquidHeight = 0;
};

constexpr u32 TextureIdInvalid = 9999;
struct LayerData
{
    u32 textureId = TextureIdInvalid;
};

struct Cell
{
    u16 areaId = 0;

    f32 heightData[CELL_TOTAL_GRID_SIZE] = { 0 };

    LiquidData liquidData;

    u16 hole = 0;

    LayerData layers[4];
};
#pragma pack(pop)