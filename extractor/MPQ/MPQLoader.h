#pragma once
#include <StormLib.h>
#include <string_view>
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <Utils/ByteBuffer.h>
#include <Utils/ConcurrentQueue.h>
#include <robin_hood.h>

class MPQLoader
{
    struct FileJob
    {
        std::string filePath;
        std::function<void(std::shared_ptr<Bytebuffer>)> callback;
    };

public:
    MPQLoader();
    ~MPQLoader();

    bool Load();
    void Close();

    // Returns nullptr if no file was found and returns the pointer to the Archive the file is in if found
    void* HasFile(std::string_view file);

    std::shared_ptr<Bytebuffer> GetFile(std::string_view file);
    void GetFileAsync(std::string_view file, std::function<void(std::shared_ptr<Bytebuffer>)> callback);

    void GetFiles(std::string pattern, std::function<void(char*, size_t)> callback);

    void __Test__();

private:
    void WorkerThread();

private:
    std::vector<void*> _archives;
    std::array<std::thread, 4> _workerThreads;
    std::atomic<bool> _isRunning = false;
    moodycamel::ConcurrentQueue<FileJob> _fileJobs;
};