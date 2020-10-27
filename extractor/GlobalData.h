#pragma once
#include <NovusTypes.h>
#include <string>
#include <filesystem>
#include <Utils/JsonConfig.h>
#include "Extractors/DBCExtractor.h"
#include "Extractors/TextureExtractor.h"
#include "Extractors/MapExtractor.h"
#include "Extractors/M2Extractor.h"

namespace fs = std::filesystem;

class GlobalData
{
public:
    GlobalData()
    {
        extractedDataPath.make_preferred();
        ndbcPath.make_preferred();
        texturePath.make_preferred();
        mapPath.make_preferred();
        wmoPath.make_preferred();
        cModelPath.make_preferred();
    }

    fs::path currentPath = fs::current_path();
    fs::path extractedDataPath = currentPath / "ExtractedData";
    fs::path ndbcPath = extractedDataPath / "Ndbc";
    fs::path texturePath = extractedDataPath / "Textures";
    fs::path mapPath = extractedDataPath / "Maps";
    fs::path wmoPath = extractedDataPath / "MapObjects";
    fs::path cModelPath = extractedDataPath / "CModels";

    std::shared_ptr<DBCExtractor> dbcExtractor = std::make_shared<DBCExtractor>();
    std::shared_ptr<TextureExtractor> textureExtractor = std::make_shared<TextureExtractor>();
    std::shared_ptr<MapExtractor> mapExtractor = std::make_shared<MapExtractor>();
    std::shared_ptr<M2Extractor> m2Extractor = std::make_shared<M2Extractor>();

    JsonConfig config;
};