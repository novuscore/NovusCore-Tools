#pragma once
#include <NovusTypes.h>

#include <Utils/DebugHandler.h>

#include "../Utils/JobBatch.h"
#include <Containers/StringTable.h>

class M2Extractor
{
public:
    void ExtractM2s(std::shared_ptr<JobBatchRunner> jobBatchRunner);
};