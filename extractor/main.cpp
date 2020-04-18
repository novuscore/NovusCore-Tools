#include <NovusTypes.h>
#include <fstream>
#include "MPQ/MPQHandler.h"
#include "DBC/DBCLoader.h"
#include "MAP/MAPLoader.h"
#include "Utils/ServiceLocator.h"

i32 main()
{
    std::shared_ptr<MPQHandler> mpqHandler = std::make_shared<MPQHandler>();
    if (mpqHandler->Load())
    {
        ServiceLocator::SetMPQHandler(mpqHandler);

        // Create Output Folders
        {
            std::filesystem::path basePath = std::filesystem::current_path();
            std::filesystem::path baseFolderPath = basePath.string() + "/ExtractedData";
            std::filesystem::path sqlFolderPath = baseFolderPath.string() + "/Sql";
            std::filesystem::path mapFolderPath = baseFolderPath.string() + "/Maps";
            if (!std::filesystem::exists(baseFolderPath))
            {
                std::filesystem::create_directory(baseFolderPath);
            }

            if (!std::filesystem::exists(sqlFolderPath))
            {
                std::filesystem::create_directory(sqlFolderPath);
            }

            if (!std::filesystem::exists(mapFolderPath))
            {
                std::filesystem::create_directory(mapFolderPath);
            }

            ServiceLocator::SetBaseFolderPath(baseFolderPath);
            ServiceLocator::SetSQLFolderPath(sqlFolderPath);
            ServiceLocator::SetMapFolderPath(mapFolderPath);
        }

        std::vector<std::string> adtLocations;
        if (DBCLoader::LoadMap(adtLocations))
        {
            MapLoader::LoadMaps(adtLocations);
        }

        DBCLoader::LoadEmotesText();
        DBCLoader::LoadSpell();

        mpqHandler->CloseAll();
        NC_LOG_SUCCESS("Finished extracting all data");
    }
    else
    {
        NC_LOG_ERROR("Failed to load any MPQs");
    }

    system("pause");
    return 0;
}
