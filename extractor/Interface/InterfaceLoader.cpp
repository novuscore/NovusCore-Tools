#include "InterfaceLoader.h"
#include "../Utils/ServiceLocator.h"
#include "../MPQ/MPQLoader.h"
#include "../Utils/JobBatch.h"
#include "../BLP/BLP2PNG/BlpConvert.h"

#include <Utils/StringUtils.h>

#include <tracy/Tracy.hpp>

void InterfaceLoader::LoadInterface(JobBatch& jobBatch)
{
    ZoneScoped;

    NC_LOG_MESSAGE("Extracting Interface...");

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    mpqLoader->GetFiles("Interface\\*.blp", [&jobBatch, mpqLoader](std::string fileName)
        {
            u32 fileNameHash = StringUtils::fnv1a_32(fileName.c_str(), fileName.size());

            std::filesystem::path outputPath = std::filesystem::current_path().append("ExtractedData/Textures").append(fileName);
            outputPath = outputPath.make_preferred().replace_extension("dds");

            std::filesystem::create_directories(outputPath.parent_path());

            jobBatch.AddJob(fileNameHash, [fileName, outputPath, mpqLoader]()
            {
                ZoneScopedN("LoadInterface::FileJob");
                    
                std::shared_ptr<Bytebuffer> buffer = mpqLoader->GetFile(fileName);

                if (buffer == nullptr) // The bytebuffer return is nullptr if the file didn't exist
                    return;

                // Convert from BLP to DDS
                BLP::BlpConvert blpConvert;
                blpConvert.ConvertBLP(buffer->GetDataPointer(), buffer->size, outputPath.string(), false);
            });
        }
    );
}
