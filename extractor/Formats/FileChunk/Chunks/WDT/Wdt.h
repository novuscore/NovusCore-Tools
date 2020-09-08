#pragma once
#include "../MVER.h"
#include "Structures.h"
#include "../ADT/Structures.h"

namespace Wdt
{
    struct Wdt
    {
        Mver mver;
        Mphd mphd;
        Main main;
        Adt::Mwmo mwmo;
        Adt::Modf modf;

        bool hasBigAlpha = false;
    };
}