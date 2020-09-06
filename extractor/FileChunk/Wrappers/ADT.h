#pragma once
#include <vector>
#include <filesystem>
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
#include <Containers/StringTable.h>

class JobBatch;

namespace fs = std::filesystem;

class GlobalData;
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
    MDDF mddf;
    MODF modf;
    MFBO mfbo;
    MH2O mh2o;
    MTXF mtxf;
    std::vector<MapCellInfo> cells;

    void SaveToDisk(std::shared_ptr<GlobalData>& globalData, const fs::path& filePath);
};