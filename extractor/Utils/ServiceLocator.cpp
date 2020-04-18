#include "ServiceLocator.h"
#include <cassert>

std::shared_ptr<MPQHandler> ServiceLocator::_mpqHandler = nullptr;
std::filesystem::path ServiceLocator::_baseFolderPath;
std::filesystem::path ServiceLocator::_sqlFolderPath;
std::filesystem::path ServiceLocator::_mapFolderPath;

void ServiceLocator::SetMPQHandler(std::shared_ptr<MPQHandler> mpqHandler)
{
    assert(mpqHandler != nullptr);
    _mpqHandler = mpqHandler;
}

void ServiceLocator::SetBaseFolderPath(std::filesystem::path baseFolderPath)
{
    assert(!baseFolderPath.empty());
    _baseFolderPath = baseFolderPath;
}

void ServiceLocator::SetSQLFolderPath(std::filesystem::path sqlFolderPath)
{
    assert(!sqlFolderPath.empty());
    _sqlFolderPath = sqlFolderPath;
}

void ServiceLocator::SetMapFolderPath(std::filesystem::path mapFolderPath)
{
    assert(!mapFolderPath.empty());
    _mapFolderPath = mapFolderPath;
}
