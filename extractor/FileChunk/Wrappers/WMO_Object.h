#pragma once
#include <vector>
#include "../Chunks/MVER.h"
#include "../Chunks/WMO/OBJECT/MOGP.h"
#include "../Chunks/WMO/OBJECT/MOPY.h"
#include "../Chunks/WMO/OBJECT/MOVI.h"
#include "../Chunks/WMO/OBJECT/MOVT.h"
#include "../Chunks/WMO/OBJECT/MONR.h"
#include "../Chunks/WMO/OBJECT/MOTV.h"
#include "../Chunks/WMO/OBJECT/MOBA.h"

class MPQFileJobBatch;

struct WMO_ROOT;
struct WMO_OBJECT
{
    MVER mver;
    MOGP mogp;
    MOPY mopy;
    MOVI movi;
    MOVT movt;
    MONR monr;
    MOTV motv;
    MOBA moba;

    WMO_ROOT* root;
    void SaveToDisk(const std::string& fileName, MPQFileJobBatch* fileJobBatch);
};