#include "WDT.h"
#include <Utils/DebugHandler.h>

WDT::WDT(std::shared_ptr<MPQFile> file, std::string fileName, std::string filePath)
    : _file(file)
    , _fileName(fileName)
    , _filePath(filePath)
{

}

std::vector<u32> WDT::Convert()
{
    std::shared_ptr<ByteBuffer> buffer = _file->buffer;

    MVER mver;
    buffer->Get<MVER>(mver);
    assert(mver.token == NOVUSMAP_MVER_TOKEN && mver.version == 18); // TODO: Order independent chunk reader -- Nix
   
    MPHD mphd;
    buffer->Get<MPHD>(mphd);

    //size_t readOffset = buffer->ReadData + mphd.size;
    MAIN main;
    buffer->Get<MAIN>(main);// , readOffset);

    std::vector<u32> adtsToRead;
    adtsToRead.reserve(NUM_SM_AREA_INFO);

    u32 ADTCount = 0;
    for (u32 i = 0; i < NUM_SM_AREA_INFO; i++)
    {
        MAIN::SMAreaInfo& areaInfo = main.mapAreaInfo[i];

        if (areaInfo.Flag_HasADT == 1)
        {
            adtsToRead.push_back(i);
        }
    }

    NC_LOG_MESSAGE("%i ADTs to read", adtsToRead.size());
    return adtsToRead;
}
