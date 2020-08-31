#include <NovusTypes.h>
#include "MPQ/MPQLoader.h"
#include "DBC/DBCLoader.h"
#include "Extractors/TextureExtractor.h"
#include "Extractors/MapExtractor.h"
#include "Extractors/M2Extractor.h"
#include "FileChunk/ChunkLoader.h"
#include "Utils/ServiceLocator.h"
#include "Utils/JobBatchRunner.h"
#include <tracy/Tracy.hpp>

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
    std::shared_ptr<MPQLoader> mpqLoader = std::make_shared<MPQLoader>();
    std::shared_ptr<ChunkLoader> chunkLoader = std::make_shared<ChunkLoader>();
    std::shared_ptr<TextureExtractor> textureExtractor = std::make_shared<TextureExtractor>();
    std::shared_ptr<MapExtractor> mapExtractor = std::make_shared<MapExtractor>();
    std::shared_ptr<M2Extractor> m2Extractor = std::make_shared<M2Extractor>();
    std::shared_ptr<DBCReader> dbcReader = std::make_shared<DBCReader>();

    std::shared_ptr<JobBatchRunner> jobBatchRunner = std::make_shared<JobBatchRunner>();
    ServiceLocator::SetMPQLoader(mpqLoader);
    ServiceLocator::SetChunkLoader(chunkLoader);
    ServiceLocator::SetDBCReader(dbcReader);
    ServiceLocator::SetTextureExtractor(textureExtractor);

    /* Runs and validaties all 280k files (3.3.5a)
        mpqLoader->__Test__();

        Example usage of GetFIle

        std::string file = "Tileset\\Generic\\Black.blp";
        std::shared_ptr<Bytebuffer> buffer = mpqLoader->GetFile(file);
        if (buffer)
        {
        
        }
    */

    if (mpqLoader->Load())
    {
        std::filesystem::path basePath = std::filesystem::current_path();
        std::filesystem::path baseFolderPath = basePath.string() + "/ExtractedData";
        std::filesystem::path sqlFolderPath = baseFolderPath.string() + "/Sql";
        std::filesystem::path ndbcFolderPath = baseFolderPath.string() + "/Ndbc";
        std::filesystem::path textureFolderPath = baseFolderPath.string() + "/Textures";
        std::filesystem::path mapFolderPath = baseFolderPath.string() + "/Maps";
        std::filesystem::path m2FolderPath = baseFolderPath.string() + "/NM2";

        // Create Output Folders
        {
            if (!std::filesystem::exists(baseFolderPath))
            {
                std::filesystem::create_directory(baseFolderPath);
            }

            if (!std::filesystem::exists(sqlFolderPath))
            {
                std::filesystem::create_directory(sqlFolderPath);
            }

            if (!std::filesystem::exists(ndbcFolderPath))
            {
                std::filesystem::create_directory(ndbcFolderPath);
            }

            if (!std::filesystem::exists(textureFolderPath))
            {
                std::filesystem::create_directory(textureFolderPath);
            }

            if (!std::filesystem::exists(mapFolderPath))
            {
                std::filesystem::create_directory(mapFolderPath);
            }

            if (!std::filesystem::exists(m2FolderPath))
            {
                std::filesystem::create_directory(m2FolderPath);
            }
        }

        jobBatchRunner->Start();
        {
            textureExtractor->ExtractTextures(jobBatchRunner);

            std::vector<std::string> internalMapNames;
            if (DBCLoader::LoadMap(internalMapNames))
            {
                mapExtractor->ExtractMaps(internalMapNames, jobBatchRunner);
            }

            m2Extractor->ExtractM2s(jobBatchRunner);

            DBCLoader::LoadEmotesText();
            DBCLoader::LoadSpell();
            
            textureExtractor->CreateTextureStringTableFile(textureFolderPath);
        }
        jobBatchRunner->Stop();

        mpqLoader->Close();
        NC_LOG_SUCCESS("Finished extracting all data");
    }
    else
    {
        NC_LOG_ERROR("Failed to load any MPQs");
    }

    system("pause");
    return 0;
}