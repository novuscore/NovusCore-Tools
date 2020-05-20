#pragma once
#include <memory>
#include <filesystem>

class MPQLoader;
class ChunkLoader;
class DBCReader;
class ServiceLocator
{
public:
    static std::shared_ptr<MPQLoader> GetMPQLoader() { return _mpqLoader; }
    static void SetMPQLoader(std::shared_ptr<MPQLoader> mpqLoader);

    static std::shared_ptr<ChunkLoader> GetChunkLoader() { return _chunkLoader; }
    static void SetChunkLoader(std::shared_ptr<ChunkLoader> chunkLoader);

    static std::shared_ptr<DBCReader> GetDBCReader() { return _dbcReader; }
    static void SetDBCReader(std::shared_ptr<DBCReader> dbcReader);

private:
    ServiceLocator() { }
    static std::shared_ptr<MPQLoader> _mpqLoader;
    static std::shared_ptr<ChunkLoader> _chunkLoader;
    static std::shared_ptr<DBCReader> _dbcReader;;
};