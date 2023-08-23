//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <Xyz/Rectangle.hpp>
#include <Yimage/Image.hpp>

namespace Tungsten
{
    struct Glyph
    {
        char32_t character;
        Xyz::RectangleF tex_rect;
        Xyz::RectangleF glyph_rect;
        Xyz::Vector2F advance;
    };

    struct Font
    {
        std::string family_name;
        unsigned size;
        std::vector<Glyph> glyphs;
        const void* image_buffer;
        size_t image_buffer_size;
    };
}
