#pragma once
#include <Containers/StringTable.h>

#include "../MVER.h"
#include "Structures.h"
#include "MapCell.h"

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

class GlobalData;
namespace Adt
{
    struct Adt
    {
        Mver mver;
        Mhdr mhdr;
        Mcin mcin;
        Mtex mtex;
        Mmdx mmdx;
        Mmid mmid;
        Mwmo mwmo;
        Mwid mwid;
        Mddf mddf;
        Modf modf;
        Mfbo mfbo;
        Mh2o mh2o;
        Mtxf mtxf;
        std::vector<MapCellInfo> cells;

        void SaveToDisk(std::shared_ptr<GlobalData>& globalData, const fs::path& filePath);
    };
}