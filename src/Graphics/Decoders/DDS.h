// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef GRAPHICS_DECODERS_DDS_H_
#define GRAPHICS_DECODERS_DDS_H_

#include "Common/Algorithm.h"
#include "Common/Logging.h"

// https://docs.microsoft.com/en-us/windows/desktop/direct3ddds/dx-graphics-dds-pguide
#define USE_DDS

namespace
{
    constexpr uint32_t kDDPFAlphaPixels = 0x1;
    constexpr uint32_t kDDPFAlpha = 0x2;
    constexpr uint32_t kDDPFFourCC = 0x4;
    constexpr uint32_t kDDPFRGB = 0x40;
    constexpr uint32_t kDDPFRGBA = kDDPFAlphaPixels | kDDPFRGB;
    constexpr uint32_t kDDPFYUV = 0x200;
    constexpr uint32_t kDDPFLuminance = 0x20000;

    constexpr uint32_t kDDSMagic = 0x20534444;  // "DDS "

    constexpr uint32_t kFourCCDXT1 = rainbow::make_fourcc('D', 'X', 'T', '1');
    constexpr uint32_t kFourCCDXT3 = rainbow::make_fourcc('D', 'X', 'T', '3');
    constexpr uint32_t kFourCCDXT5 = rainbow::make_fourcc('D', 'X', 'T', '5');

    struct DDSPixelFormat
    {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwFourCC;
        uint32_t dwRGBBitCount;
        uint32_t dwRBitMask;
        uint32_t dwGBitMask;
        uint32_t dwBBitMask;
        uint32_t dwABitMask;
    };

    struct DDSHeader
    {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwHeight;
        uint32_t dwWidth;
        uint32_t dwPitchOrLinearSize;
        uint32_t dwDepth;
        uint32_t dwMipMapCount;
        uint32_t dwReserved1[11];
        DDSPixelFormat ddspf;
        uint32_t dwCaps;
        uint32_t dwCaps2;
        uint32_t dwCaps3;
        uint32_t dwCaps4;
        uint32_t dwReserved2;
    };

    struct DDSFile
    {
        uint32_t dwMagic;
        DDSHeader header;
    };
}  // namespace

namespace dds
{
    bool check(const rainbow::Data& data)
    {
        return data.as<DDSFile*>()->dwMagic == kDDSMagic;
    }

    auto decode(const rainbow::Data& data)
    {
        using rainbow::Image;

        auto dds = data.as<DDSFile*>();
        const auto& header = dds->header;
        const auto& ddspf = header.ddspf;

        R_ASSERT((ddspf.dwFlags & kDDPFFourCC) == kDDPFFourCC,
                 "Specified DDS is missing FourCC");

        Image::Format format;
        uint32_t channels;
        switch (ddspf.dwFourCC)
        {
            case kFourCCDXT1:
                format = Image::Format::BC1;
                channels = (ddspf.dwFlags & kDDPFRGBA) == kDDPFRGBA ? 4 : 3;
                break;
            case kFourCCDXT3:
                format = Image::Format::BC2;
                channels = 4;
                break;
            case kFourCCDXT5:
                format = Image::Format::BC3;
                channels = 4;
                break;
            default:
                R_ASSERT(false, "Unsupported DDS image format");
                format = Image::Format::Unknown;
                channels = 0;
                break;
        }

        return Image(  //
            format,
            header.dwWidth,
            header.dwHeight,
            header.dwDepth,
            channels,
            header.dwPitchOrLinearSize,
            data.bytes() + sizeof(*dds));
    }
}  // namespace dds

#endif
