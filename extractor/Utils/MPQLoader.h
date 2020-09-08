#pragma once
#include <StormLib.h>
#include <vector>
#include <memory>
#include <functional>
#include <string_view>

#include <Utils/ByteBuffer.h>

class MPQLoader
{
public:
    MPQLoader();
    ~MPQLoader();

    bool Load();
    void Close();

    // Returns nullptr or pointer to the archive the file was found in
    void* HasFile(std::string_view file);

    std::shared_ptr<Bytebuffer> GetFile(std::string_view file);
    void GetFiles(std::string pattern, std::function<void(char*, size_t)> callback);

private:
    std::vector<void*> _archives;
};