#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>

struct MCNR
{
    struct MCNREntry 
    {
        i8 normal[3];
    } entries[9*9+8*8];
};