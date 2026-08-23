//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <string>
#include <Xyz/Rectangle.hpp>
#include <Yimage/Image.hpp>
#include <unordered_map>
#include <utility>

namespace Tungsten
{
    /**
     * Which point of a text's bounding rectangle sits at the origin it is
     * placed on: the left edge, the middle, or the right edge.
     */
    enum class HorizontalAnchor : uint8_t
    {
        LEFT,
        CENTER,
        RIGHT
    };

    /**
     * Which point of a text's bounding rectangle sits at the origin it is
     * placed on, vertically. BASELINE anchors the first line's baseline
     * rather than an edge of the rectangle.
     */
    enum class VerticalAnchor : uint8_t
    {
        TOP,
        CENTER,
        BOTTOM,
        BASELINE
    };

    /**
     * How the lines of a multi-line text line up with each other.
     */
    enum class HorizontalAlignment : uint8_t
    {
        LEFT,
        CENTER,
        RIGHT
    };

    struct Glyph
    {
        Xyz::RectangleF tex_rect;
        Xyz::RectangleF glyph_rect;
        float advance = 0.0;
    };

    struct FontId
    {
        std::string family;
        std::string style;
        size_t size;
    };

    inline bool operator==(const FontId& a, const FontId& b)
    {
        return a.family == b.family
               && a.style == b.style
               && a.size == b.size;
    }

    inline bool operator!=(const FontId& a, const FontId& b)
    {
        return !(a == b);
    }

    inline bool operator<(const FontId& a, const FontId& b)
    {
        if (a.family != b.family)
            return a.family < b.family;
        if (a.style != b.style)
            return a.style < b.style;
        return a.size < b.size;
    }

    struct Font
    {
        Font(FontId identifier,
             const Xyz::RectangleF& max_glyph,
             std::unordered_map<char32_t, Glyph> glyphs,
             Yimage::Image image)
            : identifier{std::move(identifier)},
              max_glyph(max_glyph),
              glyphs(std::move(glyphs)),
              image(std::move(image))
        {
        }

        FontId identifier;
        Xyz::RectangleF max_glyph;
        std::unordered_map<char32_t, Glyph> glyphs;
        Yimage::Image image;
    };

    Xyz::RectangleF get_text_rect(const Font& font,
                                  std::u32string_view text,
                                  float line_gap,
                                  bool use_full_line_height);

    Xyz::RectangleF get_text_rect(const Font& font,
                                  std::string_view text,
                                  float line_gap,
                                  bool use_full_line_height);
}
