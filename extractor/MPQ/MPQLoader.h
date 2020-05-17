#pragma once
#include <StormLib.h>
#include <string_view>
#include <memory>
#include <vector>
#include <Utils/ByteBuffer.h>

class MPQLoader
{
public:
    MPQLoader() { _archives.reserve(18); }
    ~MPQLoader() { }

    bool Load();
    void Close();

    // Returns nullptr if no file was found and returns the pointer to the Archive the file is in if found
    void* HasFile(std::string_view file);
    std::shared_ptr<ByteBuffer> GetFile(std::string_view file);
    void __Test__();
private:
    std::vector<void*> _archives;
};