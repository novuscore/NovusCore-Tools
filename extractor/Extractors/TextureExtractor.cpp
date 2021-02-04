#include "TextureExtractor.h"

#include "../GlobalData.h"
#include "../Utils/ServiceLocator.h"
#include "../Utils/JobBatchRunner.h"
#include "../Utils/MPQLoader.h"
#include "../Formats/BLP/BlpConvert.h"

#include <fstream>
#include <filesystem>
#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void TextureExtractor::ExtractTextures(std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    auto& globalData = ServiceLocator::GetGlobalData();
    json& textureConfig = globalData->config.GetJsonObjectByKey("Texture");

    // Check if we should extract the textures
    if (textureConfig["Extract"] == false)
        return;

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();

    DebugHandler::PrintSuccess("Fetching Textures");

    JobBatch textureJobBatch;

    // Load all textures into a StringTable
    mpqLoader->GetFiles("*.blp", [&](char* fileName, size_t fileNameLength)
    {
        std::string texturePathStr = fileName;
        std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

        fs::path texturePath = texturePathStr;
        fs::path outputPath = (globalData->texturePath / texturePath).replace_extension("dds").make_preferred();

        // Create Directories for the texture
        fs::create_directories(outputPath.parent_path());

        textureJobBatch.AddJob(0, [texturePath, outputPath]()
            {
                ZoneScopedN("TextureExtractor::Extract Texture");

                std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
                std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(texturePath.string());

                if (byteBuffer == nullptr || byteBuffer->size == 0) // The bytebuffer return is nullptr if the file didn't exist
                    return;

                // Convert from BLP to DDS
                BLP::BlpConvert blpConvert;
                blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
            });

    });

    DebugHandler::Print("Adding Textures batch of %u jobs", textureJobBatch.GetJobCount());

    JobBatchToken mainBatchToken = jobBatchRunner->AddBatch(textureJobBatch);
    mainBatchToken.WaitUntilFinished();
}