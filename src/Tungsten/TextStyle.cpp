//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextStyle.hpp"
#include "Tungsten/FontManager.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    constexpr uint8_t HAS_ROTATION = 0x01;
    constexpr uint8_t HAS_LINE_GAP = 0x02;
    constexpr uint8_t HAS_COLOR = 0x04;
    constexpr uint8_t HAS_HORIZONTAL_ALIGNMENT = 0x08;
    constexpr uint8_t HAS_HORIZONTAL_ANCHOR = 0x10;
    constexpr uint8_t HAS_VERTICAL_ANCHOR = 0x20;
    constexpr uint8_t HAS_ALL = 0x3F;

    TextStyle::TextStyle(std::shared_ptr<Font> font)
        : assigned_values_(HAS_ALL)
    {
        data_.font = std::move(font);
    }

    TextStyle::TextStyle(std::shared_ptr<TextStyle> parent)
        : assigned_values_(parent ? 0 : HAS_ALL),
          parent_(std::move(parent))
    {
    }

    const std::shared_ptr<TextStyle>& TextStyle::parent() const
    {
        return parent_;
    }

    TextStyle& TextStyle::set_parent(std::shared_ptr<TextStyle> parent,
                                     bool keep_values)
    {
        parent_ = std::move(parent);
        if (!keep_values)
            assigned_values_ = HAS_ALL;
        return *this;
    }

    void TextStyle::clear_all()
    {
        assigned_values_ = 0;
        data_.font = nullptr;
    }

    const std::shared_ptr<Font>& TextStyle::font() const
    {
        return data_.font;
    }

    TextStyle& TextStyle::set_font(std::shared_ptr<Font> font)
    {
        data_.font = std::move(font);
        return *this;
    }

    bool TextStyle::has_font() const
    {
        return !!data_.font;
    }

    TextStyle& TextStyle::clear_font()
    {
        data_.font = nullptr;
        return *this;
    }

    const std::shared_ptr<Font>& TextStyle::resolve_font() const // NOLINT(*-no-recursion)
    {
        if (data_.font)
            return data_.font;
        if (!parent_)
            TUNGSTEN_THROW("Style has no font.");
        return parent_->resolve_font();
    }

    float TextStyle::rotation() const
    {
        return data_.rotation;
    }

    TextStyle& TextStyle::set_rotation(float rotation)
    {
        data_.rotation = rotation;
        assigned_values_ |= HAS_ROTATION;
        return *this;
    }

    bool TextStyle::has_rotation() const
    {
        return assigned_values_ & HAS_ROTATION;
    }

    TextStyle& TextStyle::clear_rotation()
    {
        assigned_values_ &= ~HAS_ROTATION;
        return *this;
    }

    float TextStyle::resolve_rotation() const // NOLINT(*-no-recursion)
    {
        if (assigned_values_ & HAS_ROTATION)
            return data_.rotation;
        if (!parent_)
            TUNGSTEN_THROW("Style has no rotation.");
        return parent_->resolve_rotation();
    }

    float TextStyle::line_gap() const
    {
        return data_.line_gap;
    }

    TextStyle& TextStyle::set_line_gap(float gap)
    {
        data_.line_gap = gap;
        assigned_values_ |= HAS_LINE_GAP;
        return *this;
    }

    bool TextStyle::has_line_gap() const
    {
        return assigned_values_ & HAS_LINE_GAP;
    }

    TextStyle& TextStyle::clear_line_gap()
    {
        assigned_values_ &= ~HAS_LINE_GAP;
        return *this;
    }

    float TextStyle::resolve_line_gap() const // NOLINT(*-no-recursion)
    {
        if (assigned_values_ & HAS_LINE_GAP)
            return data_.line_gap;
        if (!parent_)
            TUNGSTEN_THROW("Style has no line_gap.");
        return parent_->resolve_line_gap();
    }

    Xyz::Vector4F TextStyle::color() const
    {
        return data_.color;
    }

    TextStyle& TextStyle::set_color(Xyz::Vector4F color)
    {
        data_.color = color;
        assigned_values_ |= HAS_COLOR;
        return *this;
    }

    bool TextStyle::has_color() const
    {
        return assigned_values_ & HAS_COLOR;
    }

    TextStyle& TextStyle::clear_color()
    {
        assigned_values_ &= ~HAS_COLOR;
        return *this;
    }

    Xyz::Vector4F TextStyle::resolve_color() const // NOLINT(*-no-recursion)
    {
        if (assigned_values_ & HAS_COLOR)
            return data_.color;
        if (!parent_)
            TUNGSTEN_THROW("Style has no color.");
        return parent_->resolve_color();
    }

    HorizontalAlignment TextStyle::horizontal_alignment() const
    {
        return data_.horizontal_alignment;
    }

    TextStyle& TextStyle::set_horizontal_alignment(HorizontalAlignment alignment)
    {
        data_.horizontal_alignment = alignment;
        assigned_values_ |= HAS_HORIZONTAL_ALIGNMENT;
        return *this;
    }

    bool TextStyle::has_horizontal_alignment() const
    {
        return assigned_values_ & HAS_HORIZONTAL_ALIGNMENT;
    }

    TextStyle& TextStyle::clear_horizontal_alignment()
    {
        assigned_values_ &= ~HAS_HORIZONTAL_ALIGNMENT;
        return *this;
    }

    HorizontalAlignment TextStyle::resolve_horizontal_alignment() const // NOLINT(*-no-recursion)
    {
        if (assigned_values_ & HAS_HORIZONTAL_ALIGNMENT)
            return data_.horizontal_alignment;
        if (!parent_)
            TUNGSTEN_THROW("Style has no alignment.");
        return parent_->resolve_horizontal_alignment();
    }

    HorizontalAnchor TextStyle::horizontal_anchor() const
    {
        return data_.horizontal_anchor;
    }

    TextStyle& TextStyle::set_horizontal_anchor(HorizontalAnchor anchor)
    {
        data_.horizontal_anchor = anchor;
        assigned_values_ |= HAS_HORIZONTAL_ANCHOR;
        return *this;
    }

    bool TextStyle::has_horizontal_anchor() const
    {
        return assigned_values_ & HAS_HORIZONTAL_ANCHOR;
    }

    TextStyle& TextStyle::clear_horizontal_anchor()
    {
        assigned_values_ &= ~HAS_HORIZONTAL_ANCHOR;
        return *this;
    }

    HorizontalAnchor TextStyle::resolve_horizontal_anchor() const // NOLINT(*-no-recursion)
    {
        if (assigned_values_ & HAS_HORIZONTAL_ANCHOR)
            return data_.horizontal_anchor;
        if (!parent_)
            TUNGSTEN_THROW("Style has no anchor.");
        return parent_->resolve_horizontal_anchor();
    }

    VerticalAnchor TextStyle::vertical_anchor() const
    {
        return data_.vertical_anchor;
    }

    TextStyle& TextStyle::set_vertical_anchor(VerticalAnchor anchor)
    {
        data_.vertical_anchor = anchor;
        assigned_values_ |= HAS_VERTICAL_ANCHOR;
        return *this;
    }

    bool TextStyle::has_vertical_anchor() const
    {
        return assigned_values_ & HAS_VERTICAL_ANCHOR;
    }

    TextStyle& TextStyle::clear_vertical_anchor()
    {
        assigned_values_ &= ~HAS_VERTICAL_ANCHOR;
        return *this;
    }

    VerticalAnchor TextStyle::resolve_vertical_anchor() const // NOLINT(*-no-recursion)
    {
        if (assigned_values_ & HAS_VERTICAL_ANCHOR)
            return data_.vertical_anchor;
        if (!parent_)
            TUNGSTEN_THROW("Style has no anchor.");
        return parent_->resolve_vertical_anchor();
    }
}
