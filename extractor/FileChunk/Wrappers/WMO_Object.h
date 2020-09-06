#pragma once
#include <vector>
#include <filesystem>
#include "../Chunks/MVER.h"
#include "../Chunks/WMO/OBJECT/MOGP.h"
#include "../Chunks/WMO/OBJECT/MOPY.h"
#include "../Chunks/WMO/OBJECT/MOVI.h"
#include "../Chunks/WMO/OBJECT/MOVT.h"
#include "../Chunks/WMO/OBJECT/MONR.h"
#include "../Chunks/WMO/OBJECT/MOCV.h"
#include "../Chunks/WMO/OBJECT/MOTV.h"
#include "../Chunks/WMO/OBJECT/MOBA.h"

namespace fs = std::filesystem;

class JobBatch;

struct WMO_ROOT;
struct WMO_OBJECT
{
    MVER mver;
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