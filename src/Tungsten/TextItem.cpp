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
    TextItem::TextItem(std::string text, std::shared_ptr<Font> style)
        : text_(std::move(text)),
          font_(std::move(style))
    {
    }

    TextItem::TextItem(std::string text, const TextStyleData& style)
        : text_(std::move(text)),
          font_(style.font),
          line_gap_(style.line_gap),
          color_(style.color),
          horizontal_alignment_(style.horizontal_alignment),
          horizontal_anchor_(style.horizontal_anchor),
          vertical_anchor_(style.vertical_anchor)
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

    const Xyz::Vector2F& TextItem::position() const
    {
        return position_;
    }

    void TextItem::set_position(const Xyz::Vector2F& position)
    {
        position_ = position;
    }

    float TextItem::rotation() const
    {
        return rotation_;
    }

    void TextItem::set_rotation(float rotation)
    {
        rotation_ = rotation;
    }

    Xyz::Vector2F TextItem::caclulate_size() const
    {
        return get_text_rect(*font_, text_, line_gap_, true).size;
    }
}
