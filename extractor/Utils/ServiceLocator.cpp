#include "ServiceLocator.h"
#include <cassert>

std::shared_ptr<MPQLoader> ServiceLocator::_mpqLoader = nullptr;
std::shared_ptr<ChunkLoader> ServiceLocator::_chunkLoader = nullptr;
std::shared_ptr<DBCReader> ServiceLocator::_dbcReader = nullptr;
std::shared_ptr<GlobalData> ServiceLocator::_globalData = nullptr;

std::shared_ptr<MPQLoader>& ServiceLocator::SetMPQLoader(std::shared_ptr<MPQLoader> mpqLoader)
{
    assert(mpqLoader != nullptr);
    _mpqLoader = mpqLoader;

    return _mpqLoader;
}

std::shared_ptr<ChunkLoader>& ServiceLocator::SetChunkLoader(std::shared_ptr<ChunkLoader> chunkLoader)
{
    assert(chunkLoader != nullptr);
    _chunkLoader = chunkLoader;

    return _chunkLoader;
}

std::shared_ptr<DBCReader>& ServiceLocator::SetDBCReader(std::shared_ptr<DBCReader> dbcReader)
{
    assert(dbcReader != nullptr);
    _dbcReader = dbcReader;

    return _dbcReader;
}

std::shared_ptr<GlobalData>& ServiceLocator::SetGlobalData(std::shared_ptr<GlobalData> globalData)
{
    assert(globalData != nullptr);
    _globalData = globalData;

    return _globalData;
}
