#include "ADT.h"
#include <fstream>

ADT::ADT(std::shared_ptr<MPQFile> file, std::string fileName, std::string filePath) : _file(file), _fileName(fileName), _filePath(filePath)
{
    memset(areaIds, 0, sizeof(areaIds));
    memset(heightMap, 0, sizeof(heightMap));
}

void ADT::Convert()
{

    std::shared_ptr<ByteBuffer> buffer = _file->buffer;

    MVER mver;
    buffer->Get<MVER>(mver);
    assert(mver.token == NOVUSMAP_MVER_TOKEN && mver.version == 18);

    MHDR mhdr;
    buffer->GetU32(mhdr.token);
    buffer->GetU32(mhdr.size);
    assert(mhdr.token == NOVUSMAP_MHDR_TOKEN);
    buffer->GetU32(mhdr.flags);
    buffer->GetU32(mhdr.offsetMcin);
    buffer->GetU32(mhdr.offsetMtex);
    buffer->GetU32(mhdr.offsetMmdx);
    buffer->GetU32(mhdr.offsetMmid);
    buffer->GetU32(mhdr.offsetMwmo);
    buffer->GetU32(mhdr.offsetMwid);
    buffer->GetU32(mhdr.offsetMddf);
    buffer->GetU32(mhdr.offsetModf);
    buffer->GetU32(mhdr.offsetMfbo);
    buffer->GetU32(mhdr.offsetMh2o);
    buffer->GetU32(mhdr.offsetMtfx);
    buffer->GetU32(mhdr.pad4);
    buffer->GetU32(mhdr.pad5);
    buffer->GetU32(mhdr.pad6);
    buffer->GetU32(mhdr.pad7);

    MCIN mcin;
    size_t mcinReadOffset = mhdr.offsetMcin + 0x14;
    buffer->Get<u32>(mcin.token, mcinReadOffset, true);
    assert(mcin.token == NOVUSMAP_MCIN_TOKEN);
    buffer->Get<u32>(mcin.size, mcinReadOffset, true);

    MH2O mh2o;
    size_t mh2oReadOffset = mhdr.offsetMh2o + 0x14;
    if (mhdr.offsetMh2o)
    {
        buffer->Get<u32>(mh2o.token, mh2oReadOffset, true);
        assert(mh2o.token == NOVUSMAP_MH2O_TOKEN);
        buffer->Get<u32>(mh2o.size, mh2oReadOffset, true);
    }

    ChunkHeader chunkHeader;
    NovusHeightHeader heightHeader;

    u32 mcinCellOffset = 0;
    for (u32 i = 0; i < 256; i++)
    {
        u32 y = i / 16;
        u32 x = i % 16;

        // Read MCIN Cell information
        buffer->Get<u32>(mcinCellOffset, mcinReadOffset);
        assert(mcinCellOffset < buffer->Size);
        mcinReadOffset += 0x10;

        size_t cellAddress = static_cast<size_t>(mcinCellOffset);
        
        MCNK mcnk;
        if (mcinReadOffset)
        {
            buffer->Get<u32>(mcnk.token, cellAddress, true);
            assert(mcnk.token == NOVUSMAP_MCNK_TOKEN);
            buffer->Get<u32>(mcnk.size, cellAddress, true);

            buffer->Get<u32>(mcnk.flags, cellAddress, true);
            buffer->Get<u32>(mcnk.ix, cellAddress, true);
            buffer->Get<u32>(mcnk.iy, cellAddress, true);
            buffer->Get<u32>(mcnk.layers, cellAddress, true);
            buffer->Get<u32>(mcnk.doodadRefs, cellAddress, true);
            buffer->Get<u32>(mcnk.offsetMcvt, cellAddress, true); // height map
            buffer->Get<u32>(mcnk.offsetMcnr, cellAddress, true); // Normal vectors for each vertex
            buffer->Get<u32>(mcnk.offsetMcly, cellAddress, true); // Texture layer definitions
            buffer->Get<u32>(mcnk.offsetMcrf, cellAddress, true); // A list of indices into the parent file's MDDF chunk
            buffer->Get<u32>(mcnk.offsetMcal, cellAddress, true); // Alpha maps for additional texture layers
            buffer->Get<u32>(mcnk.sizeMcal, cellAddress, true);
            buffer->Get<u32>(mcnk.offsetMcsh, cellAddress, true); // Shadow map for static shadows on the terrain
            buffer->Get<u32>(mcnk.sizeMcsh, cellAddress, true);
            buffer->Get<u32>(mcnk.areaId, cellAddress, true);
            buffer->Get<u32>(mcnk.mapObjectReferences, cellAddress, true);
            buffer->Get<u32>(mcnk.holes, cellAddress, true);

            for (u32 i = 0; i < 16; i++)
            {
                buffer->Get<u8>(mcnk.lowQualityTextureMap[i], cellAddress, true);
            }

            buffer->Get<u32>(mcnk.predTex, cellAddress, true);
            buffer->Get<u32>(mcnk.effectDoodad, cellAddress, true);
            buffer->Get<u32>(mcnk.offsetMcse, cellAddress, true);
            buffer->Get<u32>(mcnk.soundEmitters, cellAddress, true);
            buffer->Get<u32>(mcnk.offsetMclq, cellAddress, true); // Liquid level (old)
            buffer->Get<u32>(mcnk.sizeMclq, cellAddress, true);   //
            buffer->Get<f32>(mcnk.zPos, cellAddress, true);
            buffer->Get<f32>(mcnk.xPos, cellAddress, true);
            buffer->Get<f32>(mcnk.yPos, cellAddress, true);
            buffer->Get<u32>(mcnk.offsetMccv, cellAddress, true); // offsColorValues in WotLK
            buffer->Get<u32>(mcnk.props, cellAddress, true);
            buffer->Get<u32>(mcnk.effectId, cellAddress, true);

            areaIds[y][x] = mcnk.areaId;
        }


        MCVT mcvt;
        size_t mcvtReadOffset = mcnk.offsetMcvt + mcinCellOffset;
        if (mcvtReadOffset)
        {
            buffer->Get<u32>(mcvt.token, mcvtReadOffset, true);
            assert(mcvt.token == NOVUSMAP_MCVT_TOKEN);
            buffer->Get<u32>(mcvt.size, mcvtReadOffset, true);
        }

        f32 height = 0;
        for (u32 j = 0; j < 145; j++)
        {
            // Read Height Offset (if valid offset) + Add MCNK yPos
            if (mcvtReadOffset)
            {
                buffer->Get<f32>(height, mcvtReadOffset, true);
            }
            height += mcnk.yPos;

            heightMap[i][j] = height;

            if (heightHeader.gridHeight > height)
                heightHeader.gridHeight = height;
            if (heightHeader.gridMaxHeight < height)
                heightHeader.gridMaxHeight = height;
        }

        // Read MH2O Liquid Data if offset is valid
        if (mhdr.offsetMh2o)
        {
            size_t liquidHeaderOffset = mh2oReadOffset + (y * 16 + x) * sizeof(MH2O::LiquidHeader);

            MH2O::LiquidHeader header;
            buffer->Get<u32>(header.offsetInformation, liquidHeaderOffset, true);
            buffer->Get<u32>(header.layers, liquidHeaderOffset, true);
            buffer->Get<u32>(header.offsetRenderMask, liquidHeaderOffset, true);

            mh2o.liquidHeaders[y][x] = header;
        }
    }

    MFBO mfbo;
    if (mhdr.flags & 1)
    {
        size_t mfboReadOffset = mhdr.offsetMfbo + 0x14;

        buffer->Get<u32>(mfbo.token, mfboReadOffset, true);
        assert(mfbo.token == NOVUSMAP_MFBO_TOKEN);
        buffer->Get<u32>(mfbo.size, mfboReadOffset, true);

        buffer->Get(mfbo.max, mfboReadOffset, true);
        buffer->Get(mfbo.min, mfboReadOffset, true);
    }

    std::ofstream output(_filePath + "/" + _fileName, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create map file. Check admin permissions\n");
        return;
    }

    // Write Chunk Header & AreaIds
    output.write(reinterpret_cast<char const*>(&chunkHeader), sizeof(chunkHeader));
    output.write(reinterpret_cast<char const*>(&areaIds), sizeof(areaIds));

    // Write HeightHeader, Height Maps
    output.write(reinterpret_cast<char const*>(&heightHeader), sizeof(heightHeader));
    output.write(reinterpret_cast<char const*>(&heightMap), sizeof(heightMap));

    // Write Height Boxes if needed (HeightLimitBox)
    if (heightHeader.hasHeightBox)
    {
        output.write(reinterpret_cast<char*>(heightBoxMin), sizeof(heightBoxMin));
        output.write(reinterpret_cast<char*>(heightBoxMax), sizeof(heightBoxMax));
    }

    output.close();
}

u8 ADT::GetLiquidIdFromType(u16 type)
{
    switch (type)
    {
    case 1:
    case 5:
    case 9:
    case 13:
    case 17:
    case 41:
    case 61:
    case 81:
    case 181:
    {
        return 0;
    }
    case 2:
    case 6:
    case 10:
    case 14:
    case 100:
    {
        return 1;
    }
    case 3:
    case 7:
    case 11:
    case 15:
    case 19:
    case 121:
    case 141:
    {
        return 2;
    }
    case 4:
    case 8:
    case 12:
    case 20:
    case 21:
    {
        return 3;
    }
    default:
    {
        return 4;
    }
    }
}