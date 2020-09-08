#include "TextureExtractor.h"

#include "../GlobalData.h"
#include "../Utils/ServiceLocator.h"
#include "../Utils/JobBatchRunner.h"
#include "../Utils/MPQLoader.h"
#include "../Formats/BLP/BlpConvert.h"

#include <fstream>
#include <tracy/Tracy.hpp>

void TextureExtractor::ExtractTextures(std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    auto& globalData = ServiceLocator::GetGlobalData();
    json& textureConfig = globalData->config.GetJsonObjectByKey("Texture");
    
    // Check if we should completely disable extracting the textures (This should only be used for debugging)
    if (textureConfig["DisableStringTableGeneration"] == true)
        return;

    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();

    NC_LOG_SUCCESS("Fetching Textures");

    // Load all textures into a StringTable
    mpqLoader->GetFiles("*.blp", [&](char* fileName, size_t fileNameLength)
    {
        fs::path texturePath = fileName;
        texturePath.replace_extension("dds");

        std::string texturePathStr = texturePath.string();
        std::transform(texturePathStr.begin(), texturePathStr.end(), texturePathStr.begin(), ::tolower);

        _textureStringTable.AddString(texturePathStr);
    });

    // Check if we should extract the textures (We still need to build the stringtable above)
    if (textureConfig["Extract"] == false)
        return;

    JobBatch textureJobBatch;
    for (u32 i = 0; i < _textureStringTable.GetNumStrings(); i++)
    {
        const std::string& texturePath = _textureStringTable.GetString(i);

        fs::path outputPath = (globalData->texturePath / texturePath).make_preferred();
        fs::path textureFilePath = texturePath;
        textureFilePath.replace_extension("blp");

        // Create Directories for the texture
        fs::create_directories(outputPath.parent_path());

        textureJobBatch.AddJob(0, [textureFilePath, outputPath]()
        {
            ZoneScopedN("TextureExtractor::Extract Texture");

            std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
            std::shared_ptr<Bytebuffer> byteBuffer = mpqLoader->GetFile(textureFilePath.string());

            if (byteBuffer == nullptr || byteBuffer->size == 0) // The bytebuffer return is nullptr if the file didn't exist
                return;

            // Convert from BLP to DDS
            BLP::BlpConvert blpConvert;
            blpConvert.ConvertBLP(byteBuffer->GetDataPointer(), byteBuffer->size, outputPath.string(), true);
        });
    }

    NC_LOG_MESSAGE("Adding Textures batch of %u jobs", textureJobBatch.GetJobCount());

    JobBatchToken mainBatchToken = jobBatchRunner->AddBatch(textureJobBatch);
    mainBatchToken.WaitUntilFinished();
}

void TextureExtractor::CreateTextureStringTableFile()
{
    auto& globalData = ServiceLocator::GetGlobalData();
    fs::path outputPath = globalData->texturePath / "TextureStringTable.nst";

    // Create a file
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create nm2 file. Check admin permissions\n");
        return;
    }

    // Serialize and write our StringTable to the file
    std::shared_ptr<Bytebuffer> stringTableByteBuffer = Bytebuffer::Borrow<8388608>();
    _textureStringTable.Serialize(stringTableByteBuffer.get());
    output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);

    output.close();
}
