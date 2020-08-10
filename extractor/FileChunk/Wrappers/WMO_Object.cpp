#include "WMO_OBJECT.h"

#include <fstream>
#include <filesystem>

#include <Containers/StringTable.h>

#include "../../Utils/JobBatch.h"
#include "../../MAP/MapObject.h"

#include <tracy/Tracy.hpp>

namespace fs = std::filesystem;

void WMO_OBJECT::SaveToDisk(const std::string& fileName, JobBatch& jobBatch)
{
    ZoneScoped;

    // We want to convert the WMO_OBJECT to a MapObject and save it to disk
    static MapObject* mapObjectTemplate = new MapObject(); // Don't change this one, we will use it in a memcpy to "reset" object
    static MapObject* mapObject = new MapObject();

    memcpy(mapObject, mapObjectTemplate, sizeof(MapObject));

    // Convert render batches
    for (u32 i = 0; i < moba.data.size(); i++)
    {
        RenderBatch& renderBatch = mapObject->renderBatches.emplace_back();

        renderBatch.startIndex = moba.data[i].startIndex;
        renderBatch.indexCount = moba.data[i].indexCount;
        renderBatch.materialID = moba.data[i].materialId;
    }

    // Create a file
    std::ofstream output(fileName, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create MapObjectRoot file. Check admin permissions\n");
        return;
    }

    // Write header
    output.write(reinterpret_cast<char const*>(&mapObject->header), sizeof(mapObject->header));

    // Write number of indices
    u32 numIndices = static_cast<u32>(movi.indices.size());
    output.write(reinterpret_cast<char const*>(&numIndices), sizeof(u32));

    // Write indices
    if (numIndices > 0)
    {
        output.write(reinterpret_cast<char const*>(movi.indices.data()), sizeof(u16) * numIndices); // Write alpha maps
    }

    u32 numVertexPositions = static_cast<u32>(movt.vertexPosition.size());
    u32 numVertexNormals = static_cast<u32>(monr.vertexNormals.size());
    u32 numVertexUVs = static_cast<u32>(motv.vertexUVs.size());

    assert(numVertexPositions == numVertexNormals && numVertexPositions == numVertexUVs); // AFAIK, the number of these should always be the same, if this ever hits talk to Pursche

    // Write number of vertices
    output.write(reinterpret_cast<char const*>(&numVertexPositions), sizeof(u32));

    // Write vertices
    if (numVertexPositions > 0)
    {
        // Write vertexPositions
        output.write(reinterpret_cast<char const*>(movt.vertexPosition.data()), sizeof(vec3) * numVertexPositions);
        // Write vertexNormals
        output.write(reinterpret_cast<char const*>(monr.vertexNormals.data()), sizeof(vec3) * numVertexNormals);
        // Write vertexUVs
        output.write(reinterpret_cast<char const*>(motv.vertexUVs.data()), sizeof(vec2) * numVertexUVs);
    }

    // Write number of MOPYData
    u32 numMOPYData = static_cast<u32>(mopy.data.size());
    output.write(reinterpret_cast<char const*>(&numMOPYData), sizeof(u32));

    // Write MOPYData
    if (numMOPYData > 0)
    {
        output.write(reinterpret_cast<char const*>(mopy.data.data()), sizeof(MOPY::MOPYData) * numMOPYData);
    }

    // Write number of render batches
    u32 numRenderBatches = static_cast<u32>(mapObject->renderBatches.size());

    if (numRenderBatches > 0)
    {
        output.write(reinterpret_cast<char const*>(mapObject->renderBatches.data()), sizeof(RenderBatch) * numRenderBatches);
    }

    output.close();
}