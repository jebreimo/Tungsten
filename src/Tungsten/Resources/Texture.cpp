//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/Texture.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    ColorSpace to_color_space(TextureContent content, TextureSourceFormat format)
    {
        if (content == TextureContent::DATA)
            return ColorSpace::LINEAR;

        // Float colour is already linear; asking the sampler to decode it
        // again would apply the curve twice.
        if (format.type != TextureValueType::UINT8)
            return ColorSpace::LINEAR;

        switch (format.format)
        {
        case TextureFormat::RGB:
        case TextureFormat::RGBA:
            return ColorSpace::SRGB;
        default:
            TUNGSTEN_THROW("Single-channel textures cannot hold sRGB colour;"
                           " use TextureContent::DATA.");
        }
    }
} // Tungsten
