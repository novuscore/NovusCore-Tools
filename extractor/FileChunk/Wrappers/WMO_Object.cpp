#include "WMO_OBJECT.h"

#include <fstream>
#include <filesystem>

#include <Containers/StringTable.h>

#include "../../MPQ/MPQFileJobBatch.h"
#include "../../MAP/Chunk.h"
#include "../../BLP/BLP2PNG/BlpConvert.h"

#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void WMO_OBJECT::SaveToDisk(const std::string& fileName, MPQFileJobBatch* fileJobBatch)
{
    ZoneScoped;
}