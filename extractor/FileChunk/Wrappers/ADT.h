#pragma once
#include <vector>
#include "../Chunks/MVER.h"
#include "../Chunks/ADT/MHDR.h"
#include "../Chunks/ADT/MCIN.h"
#include "../Chunks/ADT/MTEX.h"
#include "../Chunks/ADT/MMDX.h"
#include "../Chunks/ADT/MMID.h"
#include "../Chunks/ADT/MWMO.h"
#include "../Chunks/ADT/MWID.h"
#include "../Chunks/ADT/MDDF.h"
#include "../Chunks/ADT/MODF.h"
#include "../Chunks/ADT/MFBO.h"
#include "../Chunks/ADT/MH2O.h"
#include "../Chunks/ADT/MTXF.h"
#include "../Chunks/ADT/MCNK.h"
#include "../Chunks/ADT/MapCell.h"

struct ADT
{
    MVER mver;
    MHDR mhdr;
    MCIN mcin;
    MTEX mtex;
    MMDX mmdx;
    MMID mmid;
    MWMO mwmo;
    MWID mwid;
    std::vector<MDDF> mddfs;
    std::vector<MODF> modfs;
    MFBO mfbo;
    MH2O mh2o;
    std::vector<MTXF> mtxfs;
    std::vector<MapCellInfo> cells;
};