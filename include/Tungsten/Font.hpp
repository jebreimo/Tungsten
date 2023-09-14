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
#include <utility>

namespace Tungsten
{
    struct Glyph
    {
        char32_t character;
        Xyz::RectangleF tex_rect;
        Xyz::RectangleF glyph_rect;
        Xyz::Vector2F advance;
    };

    struct FontId
    {
        std::string family;
        size_t size;
    };

    inline bool operator==(const FontId& a, const FontId& b)
    {
        return a.family == b.family && a.size == b.size;
    }

    inline bool operator!=(const FontId& a, const FontId& b)
    {
        return !(a == b);
    }

    inline bool operator<(const FontId& a, const FontId& b)
    {
        if (a.family != b.family)
            return a.family < b.family;
        return a.size < b.size;
    }

    struct Font
    {
        Font(FontId identifier,
             std::vector<Glyph> glyphs,
             Yimage::Image  image)
            : identifier{std::move(identifier)},
              glyphs(std::move(glyphs)),
              image(std::move(image))
        {}

        FontId identifier;
        std::vector<Glyph> glyphs;
        Yimage::Image image;
    };
}
