//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-05-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "FontUtilities.hpp"

#include "TextUtilities.hpp"

namespace Tungsten
{
    void add_quad(std::vector<GlyphVertex>& vertexes,
                  const Xyz::RectangleF& pos,
                  const Xyz::RectangleF& tex)
    {
        for (size_t i = 0; i < 4; i++)
            vertexes.emplace_back(pos[i], tex[i]);
    }

    template <typename IndexType>
    void add_quad_indexes(std::vector<IndexType>& indexes, IndexType base_index)
    {
        indexes.push_back(base_index);
        indexes.push_back(base_index + 1);
        indexes.push_back(base_index + 2);
        indexes.push_back(base_index);
        indexes.push_back(base_index + 2);
        indexes.push_back(base_index + 3);
    }

    Xyz::RectangleF add_quads(std::vector<GlyphVertex>& vertexes,
                              std::vector<int32_t>& indexes,
                              Xyz::Vector2F position,
                              std::u32string_view text,
                              const Font& font)
    {
        vertexes.reserve(vertexes.size() + text.size() * 4);
        indexes.reserve(indexes.size() + text.size() * 6);
        Xyz::RectangleF text_rect = {
            {position.x(), position.y() + font.max_glyph.origin.y()},
            {0, font.max_glyph.size.y()}
        };
        for (char32_t c : text)
        {
            auto it = font.glyphs.find(c);
            if (it == font.glyphs.end())
            {
                it = font.glyphs.find(U'?');
                if (it == font.glyphs.end())
                    continue;
            }
            const auto& glyph = it->second;
            auto g_rect = glyph.glyph_rect;
            const auto& t_rect = glyph.tex_rect;
            if (!Xyz::is_empty(g_rect))
            {
                Xyz::RectangleF rect(position + g_rect.origin, g_rect.size);
                add_quad(vertexes, rect, t_rect);
                add_quad_indexes(indexes, int32_t(vertexes.size() - 4));
                text_rect = get_union(text_rect, rect);
            }
            position.x() += glyph.advance;
        }
        return text_rect;
    }

    float get_x_factor(HorizontalAlignment horizontal_alignment)
    {
        switch (horizontal_alignment)
        {
        case HorizontalAlignment::LEFT:
            return 0.f;
        case HorizontalAlignment::CENTER:
            return -0.5f;
        case HorizontalAlignment::RIGHT:
            return -1.f;
        }
        return 0.f;
    }

    Xyz::RectangleF add_vertexes(std::vector<GlyphVertex>& vertexes,
                                 std::vector<int32_t>& indexes,
                                 const Font& font,
                                 std::u32string_view text,
                                 float line_gap,
                                 HorizontalAlignment horizontal_alignment)
    {
        if (text.empty())
            return {};

        Xyz::RectangleF text_rect;

        const auto lines = split_lines(text);
        for (size_t i = 0; i < lines.size(); i++)
        {
            const auto& line = lines[i];
            auto line_rect = get_text_rect(font, line, 0.f, true);
            float x = get_x_factor(horizontal_alignment) * line_rect.size.x();
            float y = std::ceil(-float(i) * (font.max_glyph.size.y() * (1.f + line_gap)));
            auto vertex_rect = add_quads(vertexes, indexes, {x, y}, line, font);
            if (!is_empty(vertex_rect))
            {
                vertex_rect.origin.y() = y + font.max_glyph.origin.y();
                vertex_rect.size.y() = font.max_glyph.size.y();
                text_rect = is_empty(text_rect)
                               ? vertex_rect
                               : get_union(text_rect, vertex_rect);
            }
        }
        return text_rect;
    }
}
