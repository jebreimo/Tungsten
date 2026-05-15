//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-05-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Font.hpp"
#include "Tungsten/TextStyleData.hpp"

namespace Tungsten
{
    /**
     * @brief A vertex for rendering a glyph, containing the position of the
     * glyph in screen space and the corresponding texture coordinates in the
     * font texture atlas.
     */
    using GlyphVertex = std::tuple<Xyz::Vector2F, Xyz::Vector2F>;

    Xyz::RectangleF add_vertexes(std::vector<GlyphVertex>& vertexes,
                                 std::vector<int32_t>& indexes,
                                 const Font& font,
                                 std::u32string_view text,
                                 float line_gap,
                                 HorizontalAlignment horizontal_alignment);
}
