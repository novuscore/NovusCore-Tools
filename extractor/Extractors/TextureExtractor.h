#pragma once
#include <NovusTypes.h>

#include <Utils/DebugHandler.h>

#include "../Utils/JobBatch.h"
#include <Containers/StringTable.h>

#include <filesystem>
namespace fs = std::filesystem;

class TextureExtractor
{
public:
    void ExtractTextures(std::shared_ptr<JobBatchRunner> jobBatchRunner);
    void CreateTextureStringTableFile(const fs::path& baseFolderPath);

    StringTable& GetStringTable() { return _textureStringTable; }

private:
    StringTable _textureStringTable;
};