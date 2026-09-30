//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/YimageGl.hpp"
#include <stdexcept>
#include <string>
#include <Yimage/ImageAlgorithms.hpp>
#include <Yimage/ReadImage.hpp>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        constexpr Yimage::PixelType supported_types[] = {
            Yimage::PixelType::MONO_8,
            Yimage::PixelType::RGB_8,
            Yimage::PixelType::RGBA_8
        };
    }

    TextureSourceFormat get_ogl_pixel_type(Yimage::PixelType type)
    {
        switch (type)
        {
            case Yimage::PixelType::MONO_8:
                return {TextureFormat::R, TextureValueType::UINT8};
            case Yimage::PixelType::RGB_8:
                return {TextureFormat::RGB, TextureValueType::UINT8};
            case Yimage::PixelType::RGBA_8:
                return {TextureFormat::RGBA, TextureValueType::UINT8};
            case Yimage::PixelType::MONO_ALPHA_8:
            case Yimage::PixelType::MONO_1:
            case Yimage::PixelType::MONO_2:
            case Yimage::PixelType::MONO_4:
            case Yimage::PixelType::MONO_16:
            case Yimage::PixelType::ALPHA_MONO_8:
            case Yimage::PixelType::ALPHA_MONO_16:
            case Yimage::PixelType::MONO_ALPHA_16:
            case Yimage::PixelType::RGB_16:
            case Yimage::PixelType::ARGB_8:
            case Yimage::PixelType::ARGB_16:
            case Yimage::PixelType::RGBA_16:
            default:
                TUNGSTEN_THROW("Unsupported pixel format: " + std::to_string(int(type)));
        }
    }

    Yimage::Image read_image(const std::string& file_name)
    {
        return Yimage::read_image(file_name, supported_types);
    }

    Yimage::Image read_image(const void* buffer, size_t size)
    {
        return Yimage::read_image(buffer, size, supported_types);
    }

    Xyz::Vector2I get_size(const Yimage::Image& image)
    {
        const auto view = image.view();
        return {int(view.width()), int(view.height())};
    }

    Xyz::Vector4F to_vector(Yimage::Rgba8 rgba)
    {
        constexpr float D = 255.0f;
        return {
            static_cast<float>(rgba.r) / D,
            static_cast<float>(rgba.g) / D,
            static_cast<float>(rgba.b) / D,
            static_cast<float>(rgba.a) / D
        };
    }
}
