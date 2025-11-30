//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-02-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <utility>
#include "Xyz/Vector.hpp"
#include "Yimage/Image.hpp"
#include <Tungsten/GlTypes.hpp>

namespace Tungsten
{
    [[nodiscard]]
    TextureSourceFormat get_ogl_pixel_type(Yimage::PixelType type);

    [[nodiscard]]
    Yimage::Image read_image(const std::string& file_name);

    [[nodiscard]]
    Yimage::Image read_image(const void* buffer, size_t size);

    [[nodiscard]]
    Xyz::Vector4F to_vector(Yimage::Rgba8 rgba);
}
