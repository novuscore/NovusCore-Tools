#include "Structures.h"
#include "../FileChunkHeader.h"
#include "../ADT/Adt.h"
#include "../WDT/Wdt.h"
#include "../../../../Utils/FileChunkUtils.h"

#include <tracy/Tracy.hpp>

namespace Adt
{
    bool Mhdr::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        u32 parentOffset = static_cast<u32>(buffer->readData);
        if (!buffer->Get(adt.mhdr))
        {
            assert(false);
            return false;
        }

        // Convert offsets to absolute
        if (adt.mhdr.mcin)
            adt.mhdr.mcin += parentOffset;

        if (adt.mhdr.mtex)
            adt.mhdr.mtex += parentOffset;

        if (adt.mhdr.mmdx)
            adt.mhdr.mmdx += parentOffset;

        if (adt.mhdr.mmid)
            adt.mhdr.mmid += parentOffset;

        if (adt.mhdr.mwmo)
            adt.mhdr.mwmo += parentOffset;

        if (adt.mhdr.mwid)
            adt.mhdr.mwid += parentOffset;

        if (adt.mhdr.mddf)
            adt.mhdr.mddf += parentOffset;

        if (adt.mhdr.modf)
            adt.mhdr.modf += parentOffset;

        if (adt.mhdr.mfbo)
            adt.mhdr.mfbo += parentOffset;

        if (adt.mhdr.mh2o)
            adt.mhdr.mh2o += parentOffset;

        if (adt.mhdr.mtxf)
            adt.mhdr.mtxf += parentOffset;

        return true;
    }

    bool Mcin::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        constexpr u32 McinSize = (16 * 16) * sizeof(McinData);
        assert(McinSize == header.size);

        return FileChunkUtils::LoadArrayOfStructs(buffer, McinSize, adt.mcin.data);
    }

    bool Mtex::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        adt.mtex.size = header.size;
        adt.mtex.filenames = buffer->GetReadPointer();
        buffer->readData += adt.mtex.size;

        return true;
    }
    
    bool Mmdx::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        adt.mmdx.size = header.size;
        adt.mmdx.filenames = buffer->GetReadPointer();
        buffer->readData += adt.mmdx.size;

        return true;
    }

    bool Mmid::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        adt.mmid.size = header.size;
        adt.mmid.offsets = buffer->GetReadPointer();
        buffer->readData += adt.mmid.size;

        return true;
    }

    bool Mwmo::ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt::Wdt& wdt)
    {
        wdt.mwmo.size = header.size;
        wdt.mwmo.filenames = buffer->GetReadPointer();
        buffer->readData += wdt.mwmo.size;

        return true;
    }
    bool Mwmo::ReadADT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        adt.mwmo.size = header.size;
        adt.mwmo.filenames = buffer->GetReadPointer();
        buffer->readData += adt.mwmo.size;

        return true;
    }

    bool Mwid::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        adt.mwid.size = header.size;
        adt.mwid.offset = buffer->GetReadPointer();
        buffer->readData += adt.mwid.size;

        return true;
    }

    bool Mddf::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        // Doodads are not required
        if (header.size == 0)
            return true;

        return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.mddf.data);
    }

    bool Modf::ReadWDT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, Wdt::Wdt& wdt)
    {
        Modf::ModfData& data = wdt.modf.data.emplace_back();
        if (!buffer->Get(data))
        {
            assert(false);
            return false;
        }

        return true;
    }
    bool Modf::ReadADT(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        // WMOs are not required
        if (header.size == 0)
            return true;

        return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.modf.data);
    }
    
    bool Mh2o::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        u32 parentOffset = static_cast<u32>(buffer->readData);
        constexpr u32 Mh2oHeaderSize = (16 * 16) * sizeof(Mh2oData);

        if (!FileChunkUtils::LoadArrayOfStructs(buffer, Mh2oHeaderSize, adt.mh2o.data))
        {
            assert(false);
            return false;
        }

        size_t numMh2oEntires = adt.mh2o.data.size();
        if (numMh2oEntires > 0)
        {
            std::vector<u32> attributeOffsets;
            attributeOffsets.reserve(numMh2oEntires);

            u32 numLiquidInstances = 0;
            u32 firstLiquidInstanceOffset = std::numeric_limits<u32>().max();

            adt.mh2o.headers.resize(numMh2oEntires);

            for (u32 i = 0; i < numMh2oEntires; i++)
            {
                Mh2oData& data = adt.mh2o.data[i];

                bool isFirstInstance = firstLiquidInstanceOffset == std::numeric_limits<u32>().max();
                bool hasInstanceOffset = data.instancesOffset > 0;
                bool hasAttributes = data.attributesOffset > 0;

                // Push Attribute Offset
                if (hasAttributes)
                {
                    attributeOffsets.push_back(parentOffset + data.attributesOffset);
                }

                // Increment Counter
                numLiquidInstances += data.layerCount;

                // Store First Instance Offset
                firstLiquidInstanceOffset += (parentOffset + data.instancesOffset + 1) * (isFirstInstance * hasInstanceOffset);

                // Create LiquidInfoHeader & pack data
                CellLiquidHeader& cellLiquidHeader = adt.mh2o.headers[i];
                cellLiquidHeader.packedData = hasAttributes << 7 | data.layerCount;
                cellLiquidHeader.cellID = i;
            }

            u32 numAttributes = static_cast<u32>(attributeOffsets.size());
            if (numAttributes)
            {
                adt.mh2o.attributes.resize(numAttributes);

                for (u32 i = 0; i < numAttributes; i++)
                {
                    const u32& attributeOffset = attributeOffsets[i];

                    Mh2o::LiquidAttributes& attribute = *reinterpret_cast<Mh2o::LiquidAttributes*>(&buffer->GetDataPointer()[attributeOffset]);
                    adt.mh2o.attributes[i] = attribute;
                }
            }

            if (numLiquidInstances > 0)
            {
                attributeOffsets.reserve(numMh2oEntires);

                adt.mh2o.instances.resize(numLiquidInstances);
                adt.mh2o.bitMaskForPatchesData.resize(numLiquidInstances * sizeof(u64)); // There are 8x8 patches in a cell, thus 64 bits are required to represent a state for all
                adt.mh2o.vertexData.resize(numLiquidInstances * MAX_VERTICES_PER_LIQUID_INSTANCE * sizeof(LiquidVertexFormat_Height_UV_Depth)); // Preallocate for max vertex data size, and resize at the end

                u32 totalBitMaskBytes = 0;
                u32 totalVertexDataBytes = 0;

                Mh2o::LiquidInstance* instances = reinterpret_cast<Mh2o::LiquidInstance*>(&buffer->GetDataPointer()[firstLiquidInstanceOffset]);

                for (u32 i = 0; i < numLiquidInstances; i++)
                {
                    Mh2o::LiquidInstance* instance = &instances[i];

                    bool hasBitMaskForPatches = instance->bitmapExistsOffset > 0;
                    bool hasVertexData = instance->vertexDataOffset > 0;
                    
                    CellLiquidInstance& cellLiquidInstance = adt.mh2o.instances[i];
                    cellLiquidInstance.liquidType = static_cast<u8>(instance->liquidType);
                    cellLiquidInstance.packedData = (hasVertexData << 7 | hasBitMaskForPatches << 6) | static_cast<u8>(instance->liquidVertexFormat);
                    cellLiquidInstance.heightLevel = hvec2(instance->minHeightLevel, instance->maxHeightLevel);
                    cellLiquidInstance.packedOffset = instance->yOffset << 4 | instance->xOffset;
                    cellLiquidInstance.packedSize = instance->height << 4 | instance->width;

                    if (hasBitMaskForPatches)
                    {
                        // Blizzard perfectly aligns the bits in the bitmap meaning the bits are sequencial to the used patches defined by (offsetX, offsetY, width & height)
                        u32 bitMaskBytes = (instance->width * instance->height + 7) / 8;

                        memcpy(&adt.mh2o.bitMaskForPatchesData[totalBitMaskBytes], &buffer->GetDataPointer()[parentOffset + instance->bitmapExistsOffset], bitMaskBytes);
                        totalBitMaskBytes += bitMaskBytes;
                    }

                    if (hasVertexData)
                    {
                        u32 vertexCount = (instance->width + 1) * (instance->height + 1);
                        u32 vertexDataBytes = 0;

                        // If LiquidVertexFormat == 0
                        vertexDataBytes += (vertexCount * sizeof(LiquidVertexFormat_Height_Depth)) * (instance->liquidVertexFormat == 0);

                        // If LiquidVertexFormat == 1
                        vertexDataBytes += (vertexCount * sizeof(LiquidVertexFormat_Height_UV)) * (instance->liquidVertexFormat == 1);

                        // If LiquidVertexFormat == 2
                        vertexDataBytes += (vertexCount * sizeof(LiquidVertexFormat_Depth)) * (instance->liquidVertexFormat == 2);

                        // If LiquidVertexFormat == 3
                        vertexDataBytes += (vertexCount * sizeof(LiquidVertexFormat_Height_UV_Depth)) * (instance->liquidVertexFormat == 3);

                        memcpy(&adt.mh2o.vertexData[totalVertexDataBytes], &buffer->GetDataPointer()[parentOffset + instance->vertexDataOffset], vertexDataBytes);
                        totalVertexDataBytes += vertexDataBytes;
                    }
                }

                // Resize vectors to fit with data size
                adt.mh2o.bitMaskForPatchesData.resize(totalBitMaskBytes);
                adt.mh2o.vertexData.resize(totalVertexDataBytes);
            }
        }

        size_t extraDataSize = header.size - Mh2oHeaderSize;
        buffer->readData += extraDataSize;
        return true;
    }

    bool Mcnk::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader&, const Wdt::Wdt& wdt, Adt& adt)
    {
        ZoneScoped;

        adt.cells.reserve(MAP_CELLS_PER_CHUNK);

        buffer->readData -= sizeof(FileChunkHeader);
        u32 parentOffset = static_cast<u32>(buffer->readData);

        FileChunkHeader header;
        for (i32 i = 0; i < MAP_CELLS_PER_CHUNK; i++)
        {
            MapCellInfo cell;
            if (!buffer->Get(cell.mcnk))
                return false;

            header.token = cell.mcnk.token;
            header.size = cell.mcnk.size;

            // Convert offsets to absolute (Also read Sub Chunks in here)
            {
                // Depends on version (This is for < 5.3)
                {
                    if (cell.mcnk.offsetMCVT)
                    {
                        cell.mcnk.offsetMCVT += parentOffset;

                        FileChunkHeader mcvtHeader;
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

                        FileChunkHeader mcnrHeader;
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

                    FileChunkHeader mclyHeader;
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
                        cell.mcly.data.reserve(cell.mcnk.numLayers);

                        for (u32 i = 0; i < cell.mcnk.numLayers; i++)
                        {
                            Mcly::MclyData& mclyData = cell.mcly.data.emplace_back();

                            if (!buffer->Get(mclyData, cell.mcnk.offsetMCLY + 8 + (sizeof(Mcly::MclyData) * i)))
                                return false;
                        }
                    }
                }

                if (cell.mcnk.offsetMCRF)
                {
                    cell.mcnk.offsetMCRF += parentOffset;

                    FileChunkHeader mcrfHeader;
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

                    FileChunkHeader mcalHeader;
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
                        cell.mcal.data.reserve(numLayers - 1);

                        // Go over each Layer and read alpha map
                        for (u32 i = 1; i < numLayers; i++)
                        {
                            Mcly::MclyData& mclyData = cell.mcly.data[i];
                            Mcal::McalData& mcal = cell.mcal.data.emplace_back();

                            if (wdt.hasBigAlpha)
                            {
                                // Check if compression is used (Compression is only used for Big Alpha)
                                if (mclyData.flags.CompressedAlphaMap)
                                {
                                    u8* alphaMap = &buffer->GetDataPointer()[cell.mcnk.offsetMCAL + 8 + mclyData.offsetInMCAL];
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
                                    if (!buffer->Get(mcal, cell.mcnk.offsetMCAL + 8 + mclyData.offsetInMCAL))
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
                                if (!buffer->GetBytes(&alphaMap[0], 2048, cell.mcnk.offsetMCAL + 8 + mclyData.offsetInMCAL))
                                {
                                    assert(false);
                                    return false;
                                }

                                u16 offsetOut = 0;
                                for (u16 j = 0; j < 2048; j++)
                                {
                                    u8 alpha = alphaMap[j];

                                    f32 firstVal = static_cast<f32>(alpha & 0xF);
                                    f32 secondVal = static_cast<f32>(alpha >> 4);

                                    firstVal = (firstVal / 15.0f) * 255.0f;
                                    secondVal = (secondVal / 15.0f) * 255.0f;

                                    mcal.alphaMap[offsetOut++] = static_cast<u8>(firstVal);
                                    mcal.alphaMap[offsetOut++] = static_cast<u8>(secondVal);
                                }

                                // Check if we have to fix the alpha map
                                if (!cell.mcnk.flags.DoNotFixAlphaMap)
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
                        }
                    }
                }

                if (cell.mcnk.offsetMCSH && cell.mcnk.flags.HasMCSH)
                {
                    cell.mcnk.offsetMCSH += parentOffset;

                    FileChunkHeader mcshHeader;
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
                        if (!cell.mcnk.flags.DoNotFixAlphaMap)
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

                    FileChunkHeader mcseHeader;
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

                    size_t num = mcseHeader.size / sizeof(Mcse::McseData);
                    if (num > 0)
                    {
                        cell.mcse.data.reserve(num);
                        for (size_t i = 0; i < num; i++)
                        {
                            Mcse::McseData mcseData = cell.mcse.data.emplace_back();
                            if (!buffer->Get(mcseData, cell.mcnk.offsetMCSE + 8 + (i * sizeof(Mcse::McseData))))
                            {
                                assert(false);
                                return false;
                            }
                        }
                    }
                }

                if (cell.mcnk.offsetMCLQ)
                {
                    cell.mcnk.offsetMCLQ += parentOffset;

                    FileChunkHeader mclqHeader;
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

                    FileChunkHeader mccvHeader;
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

            size_t extraDataSize = header.size - sizeof(Mcnk) + sizeof(FileChunkHeader);
            buffer->readData += extraDataSize;
            parentOffset = static_cast<u32>(buffer->readData);
        }

        return true;
    }

    bool Mfbo::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        if (!buffer->Get(adt.mfbo))
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool Mtxf::Read(std::shared_ptr<Bytebuffer>& buffer, const FileChunkHeader& header, const Wdt::Wdt& wdt, Adt& adt)
    {
        return FileChunkUtils::LoadArrayOfStructs(buffer, header.size, adt.mtxf.data);
    }
}