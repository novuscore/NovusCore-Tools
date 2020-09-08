#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

struct WMO_ROOT;
struct FileChunkHeader;
struct MOLT
{
    struct MOLTData
    {
        u8 lightType = 0;
        u8 type = 0;
        u8 useAttenuation = 0;
        u8 pad = 0;

        u32 color = 0; // This is (BGRA)
        vec3 position;
        f32 intensity = 0;

        f32 attenuationStart = 0;
        f32 attenuationnd = 0;

        f32 unk[4] = { 0 };
    };

    std::vector<MOLTData> data;

    static bool Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, WMO_ROOT& wmo);
};