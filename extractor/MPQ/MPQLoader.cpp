#include "MPQLoader.h"
#include <cassert>
#include <filesystem>
#include <chrono>
#include <Utils/StringUtils.h>

namespace fs = std::filesystem;

MPQLoader::MPQLoader()
{
    _archives.reserve(18);
    _isRunning = true;

    // Start all our worker threads
    for (std::thread& workerThread : _workerThreads)
    {
        workerThread = std::thread([this]()
        {
            WorkerThread();
        });
    }
}

MPQLoader::~MPQLoader()
{
    // Stop all our worker threads
    _isRunning = false;

    // Wait for the worker threads to stop
    for (std::thread& workerThread : _workerThreads)
    {
        workerThread.join();
    }
}

bool MPQLoader::Load()
{
    fs::path dataPath = fs::current_path().append("Data");

    // Failed to locate Data
    if (!fs::exists(dataPath))
    {
        assert(false);
        return false;
    }

    // Locate Locale Data
    const char* locales[] = 
    { 
        "frFR", "deDE", "enGB",
        "enUS", "itIT", "koKR",
        "zhCN", "zhTW", "ruRU",
        "esES", "esMX", "ptBR"
    };

    fs::path localeDataPath = fs::path();
    for (const char* locale : locales)
    {
        fs::path localePath;
        localePath.assign(dataPath);
        localePath.append(locale);

        if (fs::exists(localePath))
        {
            localeDataPath = localePath;
            break;
        }
    }

    // Failed to locate Locale Data
    if (localeDataPath.empty())
    {
        assert(false);
        return false;
    }

    /*
        Data: Exists
        Locale: Exists (We could have multiple locales, but it is up to the user to resolve this issue as MPQ data will override the previously read data if a matching file is found)
    */

    std::vector<fs::path> mpqPaths;
    std::vector<fs::path> localeMPQPaths;
    mpqPaths.reserve(18); // We reserve 18 as we know in the 3.3.5 version, there are 7 MPQs present in the data folder and 11 present in the locale folder
    localeMPQPaths.reserve(11); // We reserve 11 as we know in the 3.3.5 version, there are 11 MPQs present in the locale folder

    for (const auto& dir : fs::directory_iterator(dataPath))
    {
        if (!dir.is_regular_file())
            continue;

        fs::path path = dir.path();
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension != ".mpq")
            continue;

        mpqPaths.push_back(path);
    }
    for (const auto& dir : fs::directory_iterator(localeDataPath))
    {
        if (!dir.is_regular_file())
            continue;

        fs::path path = dir.path();
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension != ".mpq")
            continue;

        localeMPQPaths.push_back(path);
    }

    if (mpqPaths.size() == 0)
        return false;

    if (localeMPQPaths.size() == 0)
        return false;

    // Sort MPQs alphabetically
    std::sort(mpqPaths.begin(), mpqPaths.end(), [](fs::path a, fs::path b)
        {
            return a.replace_extension() < b.replace_extension();
        }
    );
    std::sort(localeMPQPaths.begin(), localeMPQPaths.end(), [](fs::path a, fs::path b)
        {
            return a.replace_extension() < b.replace_extension();
        }
    );
    mpqPaths.insert(mpqPaths.end(), localeMPQPaths.begin(), localeMPQPaths.end());
    std::reverse(mpqPaths.begin(), mpqPaths.end());
    
    for (const fs::path& mpqPath : mpqPaths)
    {
        void* mpq = nullptr;

        // Open Archive
        if (!SFileOpenArchive(mpqPath.string().c_str(), 0, 0, &mpq))
            continue;

        _archives.push_back(mpq);
    }

    return true;
}
void MPQLoader::Close()
{
    for (void* archive : _archives)
    {
        SFileCloseArchive(archive);
    }
}

void* MPQLoader::HasFile(std::string_view file)
{
    for (void* arch : _archives)
    {
        if (SFileHasFile(arch, file.data()))
            return arch;
    }

    return false;
}
std::shared_ptr<Bytebuffer> MPQLoader::GetFile(std::string_view file)
{
    std::shared_ptr<Bytebuffer> buffer = nullptr;

    void* archive = HasFile(file);
    if (!archive)
        return buffer;

    void* filePtr = nullptr;
    if (!SFileOpenFileEx(archive, file.data(), 0, &filePtr))
        return buffer;

    // We have yet to find a file larger than 200MB (Vanilla -> BFA) thus a single uint32 should be sufficient at reporting file size
    DWORD fileSize = SFileGetFileSize(filePtr, nullptr);
    if (fileSize == SFILE_INVALID_SIZE)
    {
        SFileCloseFile(filePtr);
        return buffer;
    }

    // Potentially manually allocate if we find filesize to be bigger than 200MB
    buffer = Bytebuffer::Borrow<209715200>();
    assert(static_cast<size_t>(fileSize) < buffer->size);
    buffer->size = static_cast<size_t>(fileSize);

    DWORD numBytesRead = 0;
    bool fileRead = SFileReadFile(filePtr, buffer->GetDataPointer(), fileSize, &numBytesRead, nullptr);
    SFileCloseFile(filePtr);

    if (fileRead)
    {
        buffer->writtenData = static_cast<size_t>(numBytesRead);
        assert(buffer->writtenData == buffer->size);
    }
    
    return buffer;
}

void MPQLoader::GetFileAsync(std::string_view file, std::function<void(std::shared_ptr<Bytebuffer>)> callback)
{
    FileJob fileJob;
    fileJob.filePath = file;
    fileJob.callback = callback;

    _fileJobs.enqueue(fileJob);
}

void MPQLoader::GetFiles(std::string pattern, std::function<void(std::string)> callback)
{
    std::vector<u32> foundFiles;
    foundFiles.reserve(16384); // May need to be changed in the future.

    for (void* archive : _archives)
    {
        SFILE_FIND_DATA data;

        void* searchHandle = SFileFindFirstFile(archive, pattern.c_str(), &data, nullptr);
        if (!searchHandle)
            continue;

        do
        {
            u32 nameHash = StringUtils::fnv1a_32(data.cFileName, strlen(data.cFileName));
            if (std::find(foundFiles.begin(), foundFiles.end(), nameHash) != foundFiles.end())
                continue;

            foundFiles.push_back(nameHash);
            callback(data.cFileName);
        } while (SFileFindNextFile(searchHandle, &data));

        SFileFindClose(searchHandle);
    }
}

void MPQLoader::__Test__()
{
    std::shared_ptr<Bytebuffer> buffer = nullptr;

    for (void* archive : _archives)
    {
        SFILE_FIND_DATA data;
        
        void* searchHandle = SFileFindFirstFile(archive, "*", &data, nullptr);
        if (!searchHandle)
            continue;

        do
        {
            std::shared_ptr<Bytebuffer>& buffer = GetFile(data.cFileName);
            if (!buffer)
            {
                assert(false);
            }

        } while (SFileFindNextFile(searchHandle, &data));
    }
}

void MPQLoader::WorkerThread()
{
    u32 emptyCount = 0;
    FileJob fileJob;

    while (_isRunning)
    {
        bool hadJob = _fileJobs.try_dequeue(fileJob);

        if (hadJob)
        {
            emptyCount = 0;

            std::shared_ptr<Bytebuffer> byteBuffer = GetFile(fileJob.filePath);
            fileJob.callback(byteBuffer);
        }
        else
        {
            emptyCount++;

            // If we have over 100 tries with an empty queue, we try 10 times per second
            if (emptyCount > 100)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            // If we have over 15 tries with an empty queue, we try 50 times per second
            else if (emptyCount > 15)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            // Else we try 200 times per second
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }


    }
}
