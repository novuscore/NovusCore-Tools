#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct WMO_OBJECT;
struct FileChunkHeader;
struct MONR
{
    std::vector<vec3> vertexNormals;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const WMO_ROOT& wmoRoot, WMO_OBJECT& wmoObject);
};