#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>

namespace FileChunkUtils
{
    template <typename T>
    inline bool LoadArrayOfStructs(std::shared_ptr<Bytebuffer>& buffer, u32 dataSize, std::vector<T>& container)
    {
        size_t elementSize = sizeof(T);
        size_t elementsNum = dataSize / elementSize;
        if (elementsNum == 0)
            return false;

        container.resize(elementsNum);
        if (!buffer->GetBytes(reinterpret_cast<u8*>(&container[0]), elementsNum * elementSize))
            return false;

        return true;
    }
}