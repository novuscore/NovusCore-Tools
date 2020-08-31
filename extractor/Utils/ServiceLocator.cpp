#include "ServiceLocator.h"
#include <cassert>

std::shared_ptr<MPQLoader> ServiceLocator::_mpqLoader = nullptr;
std::shared_ptr<ChunkLoader> ServiceLocator::_chunkLoader = nullptr;
std::shared_ptr<DBCReader> ServiceLocator::_dbcReader = nullptr;
std::shared_ptr<TextureExtractor> ServiceLocator::_textureExtractor = nullptr;

void ServiceLocator::SetMPQLoader(std::shared_ptr<MPQLoader> mpqLoader)
{
    assert(mpqLoader != nullptr);
    _mpqLoader = mpqLoader;
}

void ServiceLocator::SetChunkLoader(std::shared_ptr<ChunkLoader> chunkLoader)
{
    assert(chunkLoader != nullptr);
    _chunkLoader = chunkLoader;
}

void ServiceLocator::SetDBCReader(std::shared_ptr<DBCReader> dbcReader)
{
    assert(dbcReader != nullptr);
    _dbcReader = dbcReader;
}

void ServiceLocator::SetTextureExtractor(std::shared_ptr<TextureExtractor> textureExtractor)
{
    assert(textureExtractor != nullptr);
    _textureExtractor = textureExtractor;
}
