#include "MPQLoader.h"
#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;
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
std::shared_ptr<ByteBuffer> MPQLoader::GetFile(std::string_view file)
{
    std::shared_ptr<ByteBuffer> buffer = nullptr;

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
    buffer = ByteBuffer::Borrow<209715200>();
    assert(static_cast<size_t>(fileSize) < buffer->Size);
    buffer->Size = static_cast<size_t>(fileSize);

    DWORD numBytesRead = 0;
    bool fileRead = SFileReadFile(filePtr, buffer->GetDataPointer(), fileSize, &numBytesRead, nullptr);
    SFileCloseFile(filePtr);

    if (fileRead)
    {
        buffer->WrittenData = static_cast<size_t>(numBytesRead);
        assert(buffer->WrittenData == buffer->Size);
    }

    return buffer;
}

void MPQLoader::__Test__()
{
    std::shared_ptr<ByteBuffer> buffer = nullptr;

    for (void* archive : _archives)
    {
        SFILE_FIND_DATA data;
        
        void* searchHandle = SFileFindFirstFile(archive, "*", &data, nullptr);
        if (!searchHandle)
            continue;

        do
        {
            std::shared_ptr<ByteBuffer>& buffer = GetFile(data.cFileName);
            if (!buffer)
            {
                assert(false);
            }

        } while (SFileFindNextFile(searchHandle, &data));
    }
}
