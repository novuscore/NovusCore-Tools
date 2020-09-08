#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct FileChunkHeader;
struct MOPV
{
    std::vector<vec3> portalVertexList;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};