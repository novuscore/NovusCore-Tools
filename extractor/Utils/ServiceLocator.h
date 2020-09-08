#pragma once
#include <memory>
#include <filesystem>

class MPQLoader;
class FileChunkLoader;
class DBCReader;
class GlobalData;
class ServiceLocator
{
public:
    static std::shared_ptr<MPQLoader> GetMPQLoader() { return _mpqLoader; }
    static std::shared_ptr<MPQLoader>& SetMPQLoader(std::shared_ptr<MPQLoader> mpqLoader);

    static std::shared_ptr<FileChunkLoader> GetChunkLoader() { return _chunkLoader; }
    static std::shared_ptr<FileChunkLoader>& SetChunkLoader(std::shared_ptr<FileChunkLoader> chunkLoader);

    static std::shared_ptr<DBCReader> GetDBCReader() { return _dbcReader; }
    static std::shared_ptr<DBCReader>& SetDBCReader(std::shared_ptr<DBCReader> dbcReader);

    static std::shared_ptr<GlobalData> GetGlobalData() { return _globalData; }
    static std::shared_ptr<GlobalData>& SetGlobalData(std::shared_ptr<GlobalData> globalData);

private:
    ServiceLocator() { }
    static std::shared_ptr<MPQLoader> _mpqLoader;
    static std::shared_ptr<FileChunkLoader> _chunkLoader;
    static std::shared_ptr<DBCReader> _dbcReader;
    static std::shared_ptr<GlobalData> _globalData;
};