#pragma once
#include <NovusTypes.h>

#include <Utils/DebugHandler.h>

#include "../Utils/JobBatch.h"


class TextureExtractor
{
public:
    void ExtractTextures(std::shared_ptr<JobBatchRunner> jobBatchRunner);
};