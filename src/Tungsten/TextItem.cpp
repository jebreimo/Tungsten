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

    const std::shared_ptr<TextStyle>& TextItem::style() const
    {
        return style_;
    }

    void TextItem::set_style(std::shared_ptr<TextStyle> style)
    {
        style_ = std::move(style);
    }

    Xyz::Vector2F TextItem::size() const
    {
        const auto& font = style_->resolve_font();
        return get_text_rect(*font, text_, style_->line_gap(), true).size;
    }
}
