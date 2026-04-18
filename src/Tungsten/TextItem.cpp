//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextItem.hpp"
#include "TextUtilities.hpp"

namespace Tungsten
{
    TextItem::TextItem(std::string text, std::shared_ptr<TextStyle> style)
        : text_(std::move(text)),
          style_(std::move(style))
    {
    }

    const std::string& TextItem::text() const
    {
        return text_;
    }

    void TextItem::set_text(std::string text)
    {
        text_ = std::move(text);
    }

    Xyz::Vector2F TextItem::size() const
    {
        const auto& font = style_->resolve_font();
        const auto text32 = utf8_to_utf32(text_);
        Xyz::Vector2F min(MAXFLOAT, MAXFLOAT);
        Xyz::Vector2F max(-MAXFLOAT, -MAXFLOAT);
        Xyz::Vector2F pos;
        for (auto c : text32)
        {
            if (c == '\n')
            {
                pos[0] = 0;
                pos[1] -= font->max_glyph.size[1] * (1 + style_->line_gap());
                continue;
            }

            auto it = font->glyphs.find(c);
            if (it == font->glyphs.end())
            {
                if (it = font->glyphs.find('?'); it == font->glyphs.end())
                    continue;
            }

            const auto& glyph = it->second;
            if (!is_empty(glyph.glyph_rect))
            {
                const auto& bl = glyph.glyph_rect.placement.origin;
                const auto& ur = glyph.glyph_rect.placement.origin + glyph.glyph_rect.size;
                min = get_min(min, pos + bl);
                max = get_max(max, pos + ur);
            }
        }

        return max - min;
    }

    const std::shared_ptr<TextStyle>& TextItem::style() const
    {
        return style_;
    }

    void TextItem::set_style(std::shared_ptr<TextStyle> style)
    {
        style_ = std::move(style);
    }
}
