#pragma once
#include "../../MVER.h"
#include "MOGP.h"
#include "MOPY.h"
#include "MOVI.h"
#include "MOVT.h"
#include "MONR.h"
#include "MOCV.h"
#include "MOTV.h"
#include "MOBA.h"

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

struct WMO_ROOT;
struct WMO_OBJECT
{
    Mver mver;
    MOGP mogp; // Kind of a header with information about the other chunks, lots of counts
    MOPY mopy; // Material info for triangles
    MOVI movi; // Indices
    MOVT movt; // Vertex position
    MONR monr; // Vertex normals
    MOCV mocv; // Vertex colors
    MOTV motv; // Vertex uvs
    MOBA moba; // Render batches

    WMO_ROOT* root;
    void SaveToDisk(const fs::path& filePath, const WMO_ROOT& root);
};