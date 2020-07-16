#pragma once

#include <string>
#include <stdint.h>
#include <cstdlib>
#include <vector>
#include "ByteStream.h"
#include "BlpStructure.h"

namespace nvtt
{
    struct Surface;
}

namespace BLP 
{
    namespace _detail 
    {
        struct RgbDataArray 
        {
            RgbDataArray() 
            {
                data.color = 0;
            }

            union data 
            {
                uint32_t color;
                uint8_t buffer[4];
            } data;
        };
    }

    enum Format 
    {
        RGB,
        RGB_PALETTE,
        DXT1,
        DXT3,
        DXT5,
        UNKNOWN
    };

    struct Image
    {
        uint32_t width;
        uint32_t height;
        unsigned char* bytes;
    };

    class BlpConvert 
    {
        typedef void (BlpConvert::*tConvertFunction)(ByteStream&, std::vector<uint32_t>&, const std::size_t&) const;

    public:
        void Convert(unsigned char* inputBytes, std::size_t size, const std::string& outputPath, bool generateMipmaps);

    private:
        void LoadFirstLayer(const BlpHeader& header, ByteStream& data, std::vector<uint32_t>& imageData) const;

        Format GetFormat(const BlpHeader& header) const;

        void ParseUncompressed(const BlpHeader& header, ByteStream& data, std::vector<uint32_t>& imageData) const;

        void ParseUncompressedPalette(const BlpHeader& header, ByteStream& data, std::vector<uint32_t>& imageData) const;
        
        void DecompressPaletteFastPath(const BlpHeader& header, const uint32_t* palette, const std::vector<uint8_t>& indices, std::vector<uint32_t>& imageData) const;

        void DecompressPaletteARGB8(const BlpHeader& header, const uint32_t* palette, const std::vector<uint8_t>& indices, std::vector<uint32_t>& imageData) const;

        void ParseCompressed(const BlpHeader& header, ByteStream &data, std::vector<uint32_t>& imageData) const;

        void Dxt1GetBlock(ByteStream& stream, std::vector<uint32_t>& blockData, const size_t& blockOffset) const;
        void Dxt3GetBlock(ByteStream& stream, std::vector<uint32_t>& blockData, const size_t& blockOffset) const;
        void Dxt5GetBlock(ByteStream& stream, std::vector<uint32_t>& blockData, const size_t& blockOffset) const;

        void ReadDXTColors(ByteStream& stream, _detail::RgbDataArray* colors, bool preMultipliedAlpha, bool use4Colors = false) const;

        tConvertFunction GetDxtConvertFunction(const Format& format) const;

        void SwapByteOrder(uint32_t& ui) const;
    };
}
