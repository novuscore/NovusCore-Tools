#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct MCSH
{
    // These are actually 64x64 1 bit values, each bit can be on or off "black/white", the order is LSB first.
    u8 bitMask[8][8];
};