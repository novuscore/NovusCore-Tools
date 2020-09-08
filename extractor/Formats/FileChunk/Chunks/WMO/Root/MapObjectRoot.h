#pragma once
#include <Containers/StringTable.h>

#include "../../MVER.h"
#include "MOHD.h"
#include "MOTX.h"
#include "MOMT.h"
#include "MOGN.h"
#include "MOGI.h"
#include "MOSB.h"
#include "MOPV.h"
#include "MOPT.h"
#include "MOPR.h"
#include "MOLT.h"
#include "MODS.h"
#include "MODN.h"
#include "MODD.h"
#include "MFOG.h"
#include "MCVP.h"

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

struct WMO_ROOT
{
    Mver mver;
    MOHD mohd; // header file for WMO_ROOT, number of objects etc
    MOTX motx; // Texture names
    MOMT momt; // Materials
    MOGN mogn; 
    MOGI mogi; // Information for the groups
    MOSB mosb; 
    MOPV mopv;
    MOPT mopt;
    MOPR mopr;
    MOLT molt;
    MODS mods;
    MODN modn;
    MODD modd; // Information for the doodad instances
    MFOG mfog;
    MCVP mcvp;

    void SaveToDisk(const fs::path& filePath);
};