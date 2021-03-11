#include <NovusTypes.h>
#include "GlobalData.h"
#include "Utils/MPQLoader.h"
#include "Extractors/DBCExtractor.h"
#include "Extractors/TextureExtractor.h"
#include "Extractors/MapExtractor.h"
#include "Extractors/M2Extractor.h"
#include "Formats/FileChunk/FileChunkLoader.h"
#include "Utils/ServiceLocator.h"
#include "Utils/JobBatchRunner.h"
#include <Utils/JsonConfig.h>
#include <tracy/Tracy.hpp>

#include <filesystem>
namespace fs = std::filesystem;

// Test
#include "Formats/M2/M2.h"
#include "Formats/M2/ComplexModel.h"

#ifdef TRACY_ENABLE
void* operator new(std::size_t count)
{
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept
{
    TracyFree(ptr);
    free(ptr);
}

#endif

i32 main()
{
    ZoneScoped;
    std::shared_ptr<JobBatchRunner> jobBatchRunner = std::make_shared<JobBatchRunner>();

    auto& globalData = ServiceLocator::SetGlobalData(std::make_shared<GlobalData>());
    auto& mpqLoader = ServiceLocator::SetMPQLoader(std::make_shared<MPQLoader>());
    ServiceLocator::SetDBCReader(std::make_shared<DBCReader>());
    ServiceLocator::SetChunkLoader(std::make_shared<FileChunkLoader>());

    fs::path configPath = globalData->currentPath / "dataextractor.json";
    json config;

    // Default Config
    {
        config["Texture"] =
        {
            {"Extract", true}
        };

        config["Map"] =
        {
            {"Extract", true},
            {"ExtractChunkAlphaMaps", true},
            {"MapNames", json::array()}
        };

        config["M2"] =
        {
            {"Extract", true}
        };
    }

    // Load Config
    if (!globalData->config.LoadOrCreate(configPath, config))
        return 0;

    // Load all MPQ Archives
    if (!mpqLoader->Load())
    {
        DebugHandler::PrintFatal("Failed to load MPQ Files.");
        return 0;
    }

    // Create output folders
    {
        fs::create_directory(globalData->extractedDataPath);
        fs::create_directory(globalData->ndbcPath);
        fs::create_directory(globalData->texturePath);
        fs::create_directory(globalData->mapPath);
        fs::create_directory(globalData->cModelPath);
    }

    // Test
    /*{
        M2File modelFile;
        if (modelFile.GetFromMPQ("Creature\\Murloc\\Murloc.m2"))
        {
            ComplexModel complexModel;
            complexModel.ReadFromM2(modelFile);
        }
    }*/

    // Start Job Batch Runner
    jobBatchRunner->Start();
    {
        globalData->dbcExtractor->ExtractDBCs(jobBatchRunner);
        globalData->textureExtractor->ExtractTextures(jobBatchRunner);
        globalData->mapExtractor->ExtractMaps(jobBatchRunner);
        globalData->m2Extractor->ExtractM2s(jobBatchRunner);
    }
    jobBatchRunner->Stop();

    mpqLoader->Close();

    DebugHandler::PrintSuccess("Dataextractor finished");
    return 0;
}