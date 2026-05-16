//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Font.hpp"
#include "TextUtilities.hpp"
#include <string_view>

namespace Tungsten
{
    Xyz::RectangleF get_text_rect(const Font& font,
                                  std::u32string_view text,
                                  float line_gap,
                                  bool use_full_line_height)
    {
        if (text.empty())
            return {};

        Xyz::Vector2F min(MAXFLOAT, use_full_line_height ? get_min(font.max_glyph).y() : MAXFLOAT);
        Xyz::Vector2F max(-MAXFLOAT,
                          use_full_line_height ? get_max(font.max_glyph).y() : -MAXFLOAT);
        Xyz::Vector2F pos;
        for (auto c : text)
        {
            if (c == '\n')
            {
                pos[0] = 0;
                const float line_height = std::ceil(font.max_glyph.size[1] * (1 + line_gap));
                pos[1] -= line_height;
                if (use_full_line_height)
                    min.y() -= line_height;
                continue;
            }

            auto it = font.glyphs.find(c);
            if (it == font.glyphs.end())
            {
                if (it = font.glyphs.find('?'); it == font.glyphs.end())
                    continue;
            }

            const auto& glyph = it->second;
            if (!is_null(glyph.glyph_rect))
            {
                const auto& bl = glyph.glyph_rect.origin;
                const auto& ur = glyph.glyph_rect.origin + glyph.glyph_rect.size;
                min = get_min(min, pos + bl);
                max = get_max(max, pos + ur);
            }
            pos[0] += glyph.advance;
        }

        return {min, max - min};
    }

    Xyz::RectangleF get_text_rect(const Font& font,
                                  std::string_view text,
                                  float line_gap,
                                  bool use_full_line_height)
    {
        const auto text32 = utf8_to_utf32(text);
        return get_text_rect(font, text32, line_gap, use_full_line_height);
    }
}
