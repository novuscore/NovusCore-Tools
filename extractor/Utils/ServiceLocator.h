#pragma once
#include <memory>
#include <filesystem>

class MPQHandler;
class ServiceLocator
{
public:
    static std::shared_ptr<MPQHandler> GetMPQHandler() { return _mpqHandler; }
    static void SetMPQHandler(std::shared_ptr<MPQHandler> mpqHandler);

    static std::filesystem::path GetBaseFolderPath() { return _baseFolderPath; }
    static void SetBaseFolderPath(std::filesystem::path baseFolderPath);

    static std::filesystem::path GetSQLFolderPath() { return _sqlFolderPath; }
    static void SetSQLFolderPath(std::filesystem::path sqlFolderPath);

    static std::filesystem::path GetMapFolderPath() { return _mapFolderPath; }
    static void SetMapFolderPath(std::filesystem::path mapFolderPath);

private:
    ServiceLocator() { }
    static std::shared_ptr<MPQHandler> _mpqHandler;
    static std::filesystem::path _baseFolderPath;
    static std::filesystem::path _sqlFolderPath;
    static std::filesystem::path _mapFolderPath;
};