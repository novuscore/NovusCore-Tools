#include "MCNK.h"
#include "../ChunkHeader.h"
#include "../../Wrappers/WDT.h"
#include "../../Wrappers/ADT.h"

bool MCNK::Read(std::shared_ptr<ByteBuffer>& buffer, const ChunkHeader& header, const WDT& wdt, ADT& adt)
{
    // The offsets to Sub Chunks in the MCNK is relative to the start of it's own header and not to the data (This is the reason we subtract 8)
    u32 parentOffset = static_cast<u32>(buffer->ReadData) - 8;

    if (adt.cells.size() == 0)
        adt.cells.reserve(256);

    MapCellInfo cell;
    if (!buffer->Get(cell.mcnk))
    {
        assert(false);
        return false;
    }

    // Convert offsets to absolute (Also read Sub Chunks in here)
    {
        // Depends on version (This is for < 5.3)
        {
            if (cell.mcnk.offsetMCVT)
            {
                cell.mcnk.offsetMCVT += parentOffset;

                ChunkHeader mcvtHeader;
                if (!buffer->Get(mcvtHeader, cell.mcnk.offsetMCVT))
                {
                    assert(false);
                    return false;
                }

                // This should never happen
                if (mcvtHeader.token != static_cast<u32>('MCVT'))
                {
                    assert(false);
                    return false;
                }

                if (mcvtHeader.size > 0)
                {
                    if (!buffer->Get(cell.mcvt, cell.mcnk.offsetMCVT + 8))
                    {
                        assert(false);
                        return false;
                    }
                }
            }

            if (cell.mcnk.offsetMCNR)
            {
                cell.mcnk.offsetMCNR += parentOffset;

                ChunkHeader mcnrHeader;
                if (!buffer->Get(mcnrHeader, cell.mcnk.offsetMCNR))
                {
                    assert(false);
                    return false;
                }

                // This should never happen
                if (mcnrHeader.token != static_cast<u32>('MCNR'))
                {
                    assert(false);
                    return false;
                }

                if (mcnrHeader.size > 0)
                {
                    if (!buffer->Get(cell.mcnr, cell.mcnk.offsetMCNR + 8))
                    {
                        assert(false);
                        return false;
                    }
                }
            }
        }

        if (cell.mcnk.offsetMCLY)
        {
            cell.mcnk.offsetMCLY += parentOffset;

            ChunkHeader mclyHeader;
            if (!buffer->Get(mclyHeader, cell.mcnk.offsetMCLY))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mclyHeader.token != static_cast<u32>('MCLY'))
            {
                assert(false);
                return false;
            }

            if (cell.mcnk.numLayers != 0)
            {
                cell.mclys.reserve(cell.mcnk.numLayers);

                MCLY mcly;
                for (u32 i = 0; i < cell.mcnk.numLayers; i++)
                {
                    if (!buffer->Get(mcly, cell.mcnk.offsetMCLY + 8 + (sizeof(MCLY) * i)))
                        return false;

                    cell.mclys.push_back(mcly);
                }
            }
        }

        if (cell.mcnk.offsetMCRF)
        {
            cell.mcnk.offsetMCRF += parentOffset;

            ChunkHeader mcrfHeader;
            if (!buffer->Get(mcrfHeader, cell.mcnk.offsetMCRF))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mcrfHeader.token != static_cast<u32>('MCRF'))
            {
                assert(false);
                return false;
            }

            // Read DoodadRefs
            if (cell.mcnk.numDoodadRefs)
            {
                cell.mcrf.doodadRefs.reserve(cell.mcnk.numDoodadRefs);
                for (u32 i = 0; i < cell.mcnk.numDoodadRefs; i++)
                {
                    u32 doodadRef = 0;
                    if (!buffer->Get(doodadRef, cell.mcnk.offsetMCRF + 8 + (sizeof(u32) * i)))
                    {
                        assert(false);
                        return false;
                    }

                    cell.mcrf.doodadRefs.push_back(doodadRef);
                }
            }

            // Read MapObjectRefs
            if (cell.mcnk.numMapObjectRefs)
            {
                cell.mcrf.mapObjectRefs.reserve(cell.mcnk.numMapObjectRefs);
                for (u32 i = 0; i < cell.mcnk.numMapObjectRefs; i++)
                {
                    u32 mapObjectRef = 0;
                    if (!buffer->Get(mapObjectRef, cell.mcnk.offsetMCRF + 8 + (sizeof(u32) * cell.mcnk.numDoodadRefs) + (sizeof(u32) * i)))
                    {
                        assert(false);
                        return false;
                    }

                    cell.mcrf.mapObjectRefs.push_back(mapObjectRef);
                }
            }
        }

        if (cell.mcnk.offsetMCAL)
        {
            cell.mcnk.offsetMCAL += parentOffset;

            ChunkHeader mcalHeader;
            if (!buffer->Get(mcalHeader, cell.mcnk.offsetMCAL))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mcalHeader.token != static_cast<u32>('MCAL'))
            {
                assert(false);
                return false;
            }

            size_t numLayers = cell.mcnk.numLayers;

            // First layer does not pack an alpha map
            if (numLayers > 1)
            {
                cell.mcals.reserve(numLayers - 1);

                // Go over each Layer and read alpha map
                for (u32 i = 1; i < numLayers; i++)
                {
                    MCLY& mcly = cell.mclys[i];
                    MCAL mcal;

                    if (wdt.hasBigAlpha)
                    {
                        // Check if compression is used (Compression is only used for Big Alpha)
                        if ((mcly.flags & static_cast<u32>(MCLYFlags::CompressedAlphaMap)))
                        {
                            u8* alphaMap = &buffer->GetDataPointer()[cell.mcnk.offsetMCAL + 8 + mcly.offsetInMCAL];
                            u32 offsetIn = 0;
                            u32 offsetOut = 0;

                            while (offsetOut < 4096 && offsetIn < 4096)
                            {
                                bool fill = alphaMap[offsetIn] & 0x80;
                                u8 count = alphaMap[offsetIn++] & 0x7F;

                                for (u8 j = 0; j < count; j++)
                                {
                                    // According to Wowdev.wiki some compressed alphamaps are corrupt, and can generate more than
                                    // 4096 bytes of data, and therefore we must manually check for it here.
                                    if (offsetOut == 4096 || offsetIn == 4096)
                                        break;

                                    mcal.alphaMap[offsetOut++] = alphaMap[offsetIn];

                                    // Copy Mode, in this case we can to contiously copy the next sequence of bytes
                                    if (!fill)
                                        offsetIn += 1;
                                }

                                // Fill Mode, in this case we want to copy the first byte into next "count" of bytes
                                if (fill)
                                    offsetIn += 1;
                            }
                        }
                        else
                        {
                            if (!buffer->Get(mcal, cell.mcnk.offsetMCAL + 8 + mcly.offsetInMCAL))
                            {
                                assert(false);
                                return false;
                            }
                        }
                    }
                    // Check for Small Alpha, and if present we need to unpack
                    else
                    {
                        // The small alpha is 2048 bytes of data with each byte packing 2 values, the value pair starts from the least significant byte
                        // This will give us 4096 bytes of data
                        u8 alphaMap[2048];
                        if (!buffer->GetBytes(&alphaMap[0], 2048, cell.mcnk.offsetMCAL + 8 + mcly.offsetInMCAL))
                        {
                            assert(false);
                            return false;
                        }

                        u16 offsetOut = 0;
                        for (u16 j = 0; j < 2048; j++)
                        {
                            u8 firstVal = alphaMap[j] & 0xF;
                            u8 secondVal = (alphaMap[j] & 0xF0) >> 4;

                            firstVal *= 255 / 15;
                            secondVal *= 255 / 15;

                            mcal.alphaMap[offsetOut++] = firstVal;
                            mcal.alphaMap[offsetOut++] = secondVal;
                        }

                        // Check if we have to fix the alpha map
                        if (!(cell.mcnk.flags & static_cast<u32>(MCNKFlags::DoNotFixAlphaMap)))
                        {
                            // Fix last column for every row but the last
                            for (u16 j = 0; j < 4032; j += 64)
                            {
                                u8 byte = mcal.alphaMap[j + 62];
                                mcal.alphaMap[j + 63] = byte;
                            }

                            // Fix last row
                            for (u16 j = 4032; j < 4096; j++)
                            {
                                u8 byte = mcal.alphaMap[j - 64];
                                mcal.alphaMap[j] = byte;
                            }
                        }
                    }

                    cell.mcals.push_back(mcal);
                }
            }
        }

        if (cell.mcnk.offsetMCSH && (cell.mcnk.flags & static_cast<u32>(MCNKFlags::HasMCSH)))
        {
            cell.mcnk.offsetMCSH += parentOffset;

            ChunkHeader mcshHeader;
            if (!buffer->Get(mcshHeader, cell.mcnk.offsetMCSH))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mcshHeader.token != static_cast<u32>('MCSH'))
            {
                assert(false);
                return false;
            }

            if (mcshHeader.size > 0)
            {
                if (!buffer->Get(cell.mcsh, cell.mcnk.offsetMCSH + 8))
                {
                    assert(false);
                    return false;
                }

                // Check if we have to fix the shadow map
                if ((cell.mcnk.flags & static_cast<u32>(MCNKFlags::DoNotFixAlphaMap)) == 0)
                {
                    // This is packed as 8x8 u8s
                    for (int i = 0; i < 7; i++)
                    {
                        u8 byte = cell.mcsh.bitMask[i][6];
                        cell.mcsh.bitMask[i][7] = byte;
                    }

                    // Correct the last row
                    for (int j = 0; j < 8; j++)
                    {
                        u8 byte = cell.mcsh.bitMask[6][j];
                        cell.mcsh.bitMask[7][j] = byte;
                    }
                }
            }
        }

        if (cell.mcnk.offsetMCSE)
        {
            cell.mcnk.offsetMCSE += parentOffset;

            ChunkHeader mcseHeader;
            if (!buffer->Get(mcseHeader, cell.mcnk.offsetMCSE))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mcseHeader.token != static_cast<u32>('MCSE'))
            {
                assert(false);
                return false;
            }

            size_t num = mcseHeader.size / sizeof(MCSE);
            if (num > 0)
            {
                cell.mcses.reserve(num);
                for (size_t i = 0; i < num; i++)
                {
                    MCSE mcse;
                    if (!buffer->Get(mcse, cell.mcnk.offsetMCSE + 8 + (i * sizeof(MCSE))))
                    {
                        assert(false);
                        return false;
                    }

                    cell.mcses.push_back(mcse);
                }
            }
        }

        if (cell.mcnk.offsetMCLQ)
        {
            cell.mcnk.offsetMCLQ += parentOffset;

            ChunkHeader mclqHeader;
            if (!buffer->Get(mclqHeader, cell.mcnk.offsetMCLQ))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mclqHeader.token != static_cast<u32>('MCLQ'))
            {
                assert(false);
                return false;
            }

            if (mclqHeader.size > 0)
            {
                if (!buffer->Get(cell.mclq, cell.mcnk.offsetMCLQ + 8))
                {
                    assert(false);
                    return false;
                }
            }
        }

        if (cell.mcnk.offsetMCCV)
        {
            cell.mcnk.offsetMCCV += parentOffset;

            ChunkHeader mccvHeader;
            if (!buffer->Get(mccvHeader, cell.mcnk.offsetMCCV))
            {
                assert(false);
                return false;
            }

            // This should never happen
            if (mccvHeader.token != static_cast<u32>('MCCV'))
            {
                assert(false);
                return false;
            }

            if (mccvHeader.size > 0)
            {
                if (!buffer->Get(cell.mccv, cell.mcnk.offsetMCCV + 8))
                {
                    assert(false);
                    return false;
                }
            }
        }

        // Cata+ (Define struct for this later and read it)
        if (cell.mcnk.offsetMCLV)
            cell.mcnk.offsetMCLV += parentOffset;
    }

    adt.cells.push_back(cell);

    size_t extraDataSize = header.size - sizeof(MCNK);
    buffer->ReadData += extraDataSize;
    return true;
}
