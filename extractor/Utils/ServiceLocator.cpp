#include "ServiceLocator.h"
#include <cassert>

std::shared_ptr<MPQLoader> ServiceLocator::_mpqLoader = nullptr;
std::shared_ptr<DBCReader> ServiceLocator::_dbcReader = nullptr;

void ServiceLocator::SetMPQLoader(std::shared_ptr<MPQLoader> mpqLoader)
{
    assert(mpqLoader != nullptr);
    _mpqLoader = mpqLoader;
}

void ServiceLocator::SetDBCReader(std::shared_ptr<DBCReader> dbcReader)
{
    assert(dbcReader != nullptr);
    _dbcReader = dbcReader;
}
