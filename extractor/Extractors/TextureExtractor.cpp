#include "TextureExtractor.h"

#include "../Utils/ServiceLocator.h"
#include "../Utils/JobBatchRunner.h"
#include "../MPQ/MPQLoader.h"
#include "../BLP/BLP2PNG/BlpConvert.h"

#include <fstream>
#include <tracy/Tracy.hpp>

void TextureExtractor::ExtractTextures(std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    bool extractTextures = true;

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

    if (extractTextures)
    {
        JobBatch textureJobBatch;

        for (u32 i = 0; i < _textureStringTable.GetNumStrings(); i++)
        {
            const std::string& texturePath = _textureStringTable.GetString(i);
            fs::path outputPath = fs::current_path().append("ExtractedData/Textures").append(texturePath).make_preferred();

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
}

void TextureExtractor::CreateTextureStringTableFile(const fs::path& baseFolderPath)
{
    fs::path outputPath = baseFolderPath / "TextureStringTable.nst";

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
