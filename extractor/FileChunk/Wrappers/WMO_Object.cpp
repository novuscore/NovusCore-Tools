#include "WMO_Object.h"
#include "WMO_Root.h"

#include <fstream>
#include <filesystem>

#include <Containers/StringTable.h>

#include "../../Utils/JobBatch.h"
#include "../../MAP/MapObject.h"

#include <tracy/Tracy.hpp>
#include <Utils/StringUtils.h>

namespace fs = std::filesystem;

void WMO_OBJECT::SaveToDisk(const std::string& fileName, const WMO_ROOT& root, JobBatch& jobBatch)
{
    ZoneScoped;

    // We want to convert the WMO_OBJECT to a MapObject and save it to disk
    static MapObject* mapObjectTemplate = new MapObject(); // Don't change this one, we will use it in a memcpy to "reset" object
    thread_local MapObject* mapObject = new MapObject();

    memcpy(mapObject, mapObjectTemplate, sizeof(MapObject));

    // Convert render batches
    for (u32 i = 0; i < moba.data.size(); i++)
    {
        RenderBatch& renderBatch = mapObject->renderBatches.emplace_back();

        renderBatch.startIndex = moba.data[i].startIndex;
        renderBatch.indexCount = moba.data[i].indexCount;
        renderBatch.materialID = moba.data[i].materialId;
    }

    // Fix vertex positions
    for (u32 i = 0; i < movt.vertexPosition.size(); i++)
    {
        vec3 pos = movt.vertexPosition[i];

        pos = vec3(-pos.x, pos.z, -pos.y);

        movt.vertexPosition[i] = pos;
    }

    // Fix vertex normals
    for (u32 i = 0; i < monr.vertexNormals.size(); i++)
    {
        vec3 normal = monr.vertexNormals[i];

        normal = vec3(normal.x, normal.z, normal.y);

        monr.vertexNormals[i] = normal;
    }

    // Fix vertex colors
    if (mocv.data.size() > 0 && !mocv.data[0].isAlphaOnly)
    {
        if (mocv.data[0].vertexColors.size() > 0)
        {
            u32 endOfTransBatch = 0;

            if (mogp.transBatchCount > 0)
            {
                endOfTransBatch = moba.data[mogp.transBatchCount - 1u].lastVertex + 1;
            }

            bool hasOutdoorsGroup = root.mohd.flags & 0x08;
            if (hasOutdoorsGroup)
            {
                for (u32 i = endOfTransBatch; i < mocv.data[0].vertexColors.size(); i++)
                {
                    mocv.data[0].vertexColors[i].a = mogp.flags & 0x08 ? 255 : 0;
                }
            }
            else
            {
                // First batch
                for (u32 i = 0; i < endOfTransBatch; i++)
                {
                    f32 b = static_cast<f32>(mocv.data[0].vertexColors[i].b) / 255.0f;
                    f32 g = static_cast<f32>(mocv.data[0].vertexColors[i].g) / 255.0f;
                    f32 r = static_cast<f32>(mocv.data[0].vertexColors[i].r) / 255.0f;
                    f32 a = static_cast<f32>(mocv.data[0].vertexColors[i].a) / 255.0f;

                    f32 ambientB = static_cast<f32>(root.mohd.ambientColor.b) / 255.0f;
                    f32 ambientG = static_cast<f32>(root.mohd.ambientColor.g) / 255.0f;
                    f32 ambientR = static_cast<f32>(root.mohd.ambientColor.r) / 255.0f;

                    // Subtract ambientColor.bgr
                    b -= ambientB;
                    g -= ambientG;
                    r -= ambientR;

                    b = b * (1.0f - a);
                    g = g * (1.0f - a);
                    r = r * (1.0f - a);

                    // Apply it back clamped
                    mocv.data[0].vertexColors[i].b = static_cast<u8>(Math::Clamp(b * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].g = static_cast<u8>(Math::Clamp(g * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].r = static_cast<u8>(Math::Clamp(r * 255.0f, 0.0f, 255.0f));
                }

                // The second two batches
                for (u32 i = endOfTransBatch; i < mocv.data[0].vertexColors.size(); i++)
                {
                    f32 b = static_cast<f32>(mocv.data[0].vertexColors[i].b) / 255.0f;
                    f32 g = static_cast<f32>(mocv.data[0].vertexColors[i].g) / 255.0f;
                    f32 r = static_cast<f32>(mocv.data[0].vertexColors[i].r) / 255.0f;
                    f32 a = static_cast<f32>(mocv.data[0].vertexColors[i].a) / 255.0f;

                    f32 ambientB = static_cast<f32>(root.mohd.ambientColor.b) / 255.0f;
                    f32 ambientG = static_cast<f32>(root.mohd.ambientColor.g) / 255.0f;
                    f32 ambientR = static_cast<f32>(root.mohd.ambientColor.r) / 255.0f;

                    b += ((b * a / 64.0f) - ambientB);
                    g += ((g * a / 64.0f) - ambientG);
                    r += ((r * a / 64.0f) - ambientR);

                    // Apply it back clamped
                    mocv.data[0].vertexColors[i].b = static_cast<u8>(Math::Clamp(b * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].g = static_cast<u8>(Math::Clamp(g * 255.0f, 0.0f, 255.0f));
                    mocv.data[0].vertexColors[i].r = static_cast<u8>(Math::Clamp(r * 255.0f, 0.0f, 255.0f));
                }
            }
        }
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

    // Write flags
    output.write(reinterpret_cast<char const*>(&mogp.flags), sizeof(u32));

    // Write number of indices
    u32 numIndices = static_cast<u32>(movi.indices.size());
    output.write(reinterpret_cast<char const*>(&numIndices), sizeof(u32));

    // Write indices
    if (numIndices > 0)
    {
        output.write(reinterpret_cast<char const*>(movi.indices.data()), sizeof(u16) * numIndices);
    }

    u32 numVertexPositions = static_cast<u32>(movt.vertexPosition.size());
    u32 numVertexNormals = static_cast<u32>(monr.vertexNormals.size());

    assert(numVertexPositions == numVertexNormals); // AFAIK, the number of these should always be the same, if this ever hits talk to Pursche
    
    // Write number of vertices
    output.write(reinterpret_cast<char const*>(&numVertexPositions), sizeof(u32));

    // Write vertices
    if (numVertexPositions > 0)
    {
        // Write vertexPositions
        output.write(reinterpret_cast<char const*>(movt.vertexPosition.data()), sizeof(vec3) * numVertexPositions);
        // Write vertexNormals
        output.write(reinterpret_cast<char const*>(monr.vertexNormals.data()), sizeof(vec3) * numVertexNormals);
    }

    // Write number of vertex color sets
    u32 numVertexColorSets = static_cast<u32>(mocv.data.size());
    output.write(reinterpret_cast<char const*>(&numVertexColorSets), sizeof(u32));

    for (u32 i = 0; i < numVertexColorSets; i++)
    {
        // Write number of vertex colors
        u32 numVertexColors = static_cast<u32>(mocv.data[i].vertexColors.size());
        output.write(reinterpret_cast<char const*>(&numVertexColors), sizeof(u32));

        // Write vertex colors
        if (numVertexColors > 0)
        {
            assert(numVertexPositions == numVertexColors); // AFAIK, if there are vertex colors there should always be the same number of them as vertex positions, if this ever hits talk to Pursche

            // Write vertexColors
            output.write(reinterpret_cast<char const*>(mocv.data[i].vertexColors.data()), sizeof(IntColor) * numVertexColors);
        }
    }

    // Write number of UV sets
    u32 numUVSets = static_cast<u32>(motv.data.size());
    output.write(reinterpret_cast<char const*>(&numUVSets), sizeof(u32));

    for (MOTV::MOTVData& motvData : motv.data)
    {
        u32 numVertexUVs = static_cast<u32>(motvData.vertexUVs.size());

        assert(numVertexPositions == numVertexUVs); // AFAIK, the number of these should always be the same, if this ever hits talk to Pursche

        // Write vertexUVs
        output.write(reinterpret_cast<char const*>(motvData.vertexUVs.data()), sizeof(vec2) * numVertexUVs);
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
    output.write(reinterpret_cast<char const*>(&numRenderBatches), sizeof(u32));

    if (numRenderBatches > 0)
    {
        output.write(reinterpret_cast<char const*>(mapObject->renderBatches.data()), sizeof(RenderBatch) * numRenderBatches);
    }

    output.close();
}