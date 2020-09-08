#include "DBCReader.h"

/*
	1 = Can't open file
	2 = Invalid format
	3 = Invalid data / string size read
*/

int DBCReader::Load(std::shared_ptr<Bytebuffer> buffer)
{
    u32 header = 0;
    buffer->GetU32(header);

    // Check for WDBC header
    if (header != NOVUSDBC_WDBC_TOKEN)
        return 2;

    try
    {
        buffer->GetU32(_rowCount);
        buffer->GetU32(_fieldCount);
        buffer->GetU32(_rowSize);
        buffer->GetU32(_stringSize);
    }
    catch (std::exception)
    {
        return 2;
    }
     
    // Cleanup Memory if we've previously loaded DBC Files
    if (_data)
    {
        delete[] _data;
    }

    u32 dataSize = _rowSize * _rowCount + _stringSize;
    _data = new unsigned char[dataSize];
    std::memset(_data, 0, dataSize);
    _stringTable = _data + _rowSize * _rowCount;

    if (!_data || !_stringTable)
        return 3;

    buffer->GetBytes(_data, dataSize);
    return 0;
}

DBCReader::DBCRow DBCReader::GetRow(u32 id)
{
    return DBCRow(*this, _data + id * _rowSize);
}