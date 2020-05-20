#pragma once
#include <vector>
#include "MCNK.h"
#include "MCVT.h"
#include "MCNR.h"
#include "MCLY.h"
#include "MCRF.h"
#include "MCAL.h"
#include "MCSH.h"
#include "MCSE.h"
#include "MCLQ.h"
#include "MCCV.h"

struct MapCellInfo
{
    MCNK mcnk;
    MCVT mcvt;
    MCNR mcnr;
    std::vector<MCLY> mclys;
    MCRF mcrf;
    std::vector<MCAL> mcals;
    MCSH mcsh;
    std::vector<MCSE> mcses;
    MCLQ mclq;
    MCCV mccv;
};