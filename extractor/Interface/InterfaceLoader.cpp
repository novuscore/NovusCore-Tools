#include "InterfaceLoader.h"
#include "../Utils/ServiceLocator.h"
#include "../MPQ/MPQLoader.h"
#include "../MPQ/MPQFileJobBatch.h"
#include "../BLP/BLP2PNG/BlpConvert.h"

#include <tracy/Tracy.hpp>

void InterfaceLoader::LoadInterface()
{
    ZoneScoped;

    NC_LOG_MESSAGE("Extracting Interface...");

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    MPQFileJobBatch mpqFileJob;

    mpqLoader->GetFiles("Interface\\*.blp", [&mpqFileJob](std::string fileName)
        {
            mpqFileJob.AddFileJob(fileName, [fileName](std::shared_ptr<Bytebuffer> buffer)
                {
                    ZoneScopedN("LoadInterface::FileJob");

                    std::filesystem::path outputPath = std::filesystem::current_path().append("ExtractedData/Textures").append(fileName);
                    outputPath = outputPath.make_preferred().replace_extension("dds");

                    std::filesystem::create_directories(outputPath.parent_path());

                    // Convert from BLP to DDS
                    BLP::BlpConvert blpConvert;
                    blpConvert.Convert(buffer->GetDataPointer(), buffer->size, outputPath.string(), false);
                });
        }
    );

    mpqFileJob.Process();
}
