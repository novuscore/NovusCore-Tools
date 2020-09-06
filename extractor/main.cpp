#include <NovusTypes.h>
#include "GlobalData.h"
#include "MPQ/MPQLoader.h"
#include "Extractors/DBCExtractor.h"
#include "Extractors/TextureExtractor.h"
#include "Extractors/MapExtractor.h"
#include "Extractors/M2Extractor.h"
#include "FileChunk/ChunkLoader.h"
#include "Utils/ServiceLocator.h"
#include "Utils/JobBatchRunner.h"
#include <Utils/JsonConfig.h>
#include <tracy/Tracy.hpp>

#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

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
    ServiceLocator::SetChunkLoader(std::make_shared<ChunkLoader>());

    /* Runs and validaties all 280k files (3.3.5a)
        mpqLoader->__Test__();

        Example usage of GetFIle

        std::string file = "Tileset\\Generic\\Black.blp";
        std::shared_ptr<Bytebuffer> buffer = mpqLoader->GetFile(file);
        if (buffer)
        {

        }
    */

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

    /* Load Database Config Handler for server */
    if (!globalData->config.LoadOrCreate(configPath, config))
        return 0;

    if (!mpqLoader->Load())
    {
        NC_LOG_FATAL("Failed to load MPQ Files.");
        return 0;
    }


    // Create output folders
    {
        fs::create_directory(globalData->extractedDataPath);
        fs::create_directory(globalData->ndbcPath);
        fs::create_directory(globalData->texturePath);
        fs::create_directory(globalData->mapPath);
        fs::create_directory(globalData->nm2Path);
    }

    // Start Job Batch Runner
    jobBatchRunner->Start();
    {
        globalData->dbcExtractor->ExtractDBCs(jobBatchRunner);
        globalData->textureExtractor->ExtractTextures(jobBatchRunner);
        {
            globalData->mapExtractor->ExtractMaps(jobBatchRunner);
            globalData->m2Extractor->ExtractM2s(jobBatchRunner);
        }
        globalData->textureExtractor->CreateTextureStringTableFile();
    }
    jobBatchRunner->Stop();

    mpqLoader->Close();

    NC_LOG_SUCCESS("Dataextractor finished");
    return 0;
}