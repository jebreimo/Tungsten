//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/TextItem.hpp"
#include "Tungsten/Render/TextUtilities.hpp"

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
        if (text != text_)
        {
            dirty_flags_ |= DirtyFlags::TEXT;
            text_ = std::move(text);
        }
    }

    const Xyz::Vector2F& TextItem::position() const
    {
        return position_;
    }

    void TextItem::set_position(const Xyz::Vector2F& position)
    {
        if (position != position_)
        {
            dirty_flags_ |= DirtyFlags::POSITION;
            position_ = position;
        }
    }

    float TextItem::rotation() const
    {
        return rotation_;
    }

    void TextItem::set_rotation(float rotation)
    {
        if (rotation != rotation_)
        {
            dirty_flags_ |= DirtyFlags::ROTATION;
            rotation_ = rotation;
        }
    }

    const std::shared_ptr<Font>& TextItem::font() const
    {
        return font_;
    }

    void TextItem::set_font(std::shared_ptr<Font> font)
    {
        if (font != font_)
        {
            dirty_flags_ |= DirtyFlags::FONT;
            font_ = std::move(font);
        }
    }

    float TextItem::line_gap() const
    {
        return line_gap_;
    }

    void TextItem::set_line_gap(float gap)
    {
        if (gap != line_gap_)
        {
            dirty_flags_ |= DirtyFlags::LINE_GAP;
            line_gap_ = gap;
        }
    }

    const Xyz::Vector4F& TextItem::color() const
    {
        return color_;
    }

    void TextItem::set_color(const Xyz::Vector4F& color)
    {
        if (color != color_)
        {
            dirty_flags_ |= DirtyFlags::COLOR;
            color_ = color;
        }
    }

    HorizontalAlignment TextItem::horizontal_alignment() const
    {
        return horizontal_alignment_;
    }

    void TextItem::set_horizontal_alignment(HorizontalAlignment alignment)
    {
        if (alignment != horizontal_alignment_)
        {
            dirty_flags_ |= DirtyFlags::HORIZONTAL_ALIGNMENT;
            horizontal_alignment_ = alignment;
        }
    }

    HorizontalAnchor TextItem::horizontal_anchor() const
    {
        return horizontal_anchor_;
    }

    void TextItem::set_horizontal_anchor(HorizontalAnchor anchor)
    {
        if (anchor != horizontal_anchor_)
        {
            dirty_flags_ |= DirtyFlags::HORIZONTAL_ANCHOR;
            horizontal_anchor_ = anchor;
        }
    }

    VerticalAnchor TextItem::vertical_anchor() const
    {
        return vertical_anchor_;
    }

    void TextItem::set_vertical_anchor(VerticalAnchor anchor)
    {
        if (anchor != vertical_anchor_)
        {
            dirty_flags_ |= DirtyFlags::VERTICAL_ANCHOR;
            vertical_anchor_ = anchor;
        }
    }

    bool TextItem::is_visible() const
    {
        return visible_;
    }

    void TextItem::set_visible(bool visible)
    {
        if (visible != visible_)
        {
            dirty_flags_ |= DirtyFlags::VISIBLE;
            visible_ = visible;
        }
    }

    Xyz::Vector2F TextItem::calc_size() const
    {
        return get_text_rect(*font_, text_, line_gap_, true).size;
    }

    TextItem::DirtyFlags TextItem::dirty_flags() const
    {
        return dirty_flags_;
    }

    void TextItem::set_dirty_flags(DirtyFlags flags)
    {
        dirty_flags_ = flags;
    }
}
