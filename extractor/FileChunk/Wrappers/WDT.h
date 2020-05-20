#pragma once
#include "../Chunks/MVER.h"
#include "../Chunks/WDT/MPHD.h"
#include "../Chunks/WDT/MAIN.h"
#include "../Chunks/ADT/MWMO.h"
#include "../Chunks/ADT/MODF.h"

struct WDT
{
    MVER mver;
    MPHD mphd;
    MAIN main;
    MWMO mwmo;
    MODF modf;

    bool hasBigAlpha = false;
};