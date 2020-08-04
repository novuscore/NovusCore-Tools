#pragma once
#include <vector>
#include "../Chunks/MVER.h"
#include "../Chunks/WMO/ROOT/MOHD.h"
#include "../Chunks/WMO/ROOT/MOTX.h"
#include "../Chunks/WMO/ROOT/MOMT.h"
#include "../Chunks/WMO/ROOT/MOGN.h"
#include "../Chunks/WMO/ROOT/MOGI.h"
#include "../Chunks/WMO/ROOT/MOSB.h"
#include "../Chunks/WMO/ROOT/MOPV.h"
#include "../Chunks/WMO/ROOT/MOPT.h"
#include "../Chunks/WMO/ROOT/MOPR.h"
#include "../Chunks/WMO/ROOT/MOLT.h"
#include "../Chunks/WMO/ROOT/MODS.h"
#include "../Chunks/WMO/ROOT/MODN.h"
#include "../Chunks/WMO/ROOT/MODD.h"
#include "../Chunks/WMO/ROOT/MFOG.h"
#include "../Chunks/WMO/ROOT/MCVP.h"

class MPQFileJobBatch;

struct WMO_ROOT
{
    MVER mver;
    MOHD mohd;
    MOTX motx;
    MOMT momt;
    MOGN mogn;
    MOGI mogi;
    MOSB mosb;
    MOPV mopv;
    MOPT mopt;
    MOPR mopr;
    MOLT molt;
    MODS mods;
    MODN modn;
    MODD modd;
    MFOG mfog;
    MCVP mcvp;

    void SaveToDisk(const std::string& fileName, MPQFileJobBatch* fileJobBatch);
};