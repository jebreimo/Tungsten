//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer2.hpp"

#include <unordered_set>

namespace Tungsten
{
    struct TextRenderer2::Data
    {
        std::shared_ptr<Font> font_;
        Xyz::Vector2F position_;
        Xyz::Vector2F size_;
        HorizontalAnchor horizontal_anchor_ = HorizontalAnchor::LEFT;
        VerticalAnchor vertical_anchor_ = VerticalAnchor::TOP;
        std::unordered_set<std::shared_ptr<TextItem>> text_entries_;
    };

    TextRenderer2::TextRenderer2(Xyz::Vector2F position)
        : data_(std::make_unique<Data>())
    {
    }

    TextRenderer2::~TextRenderer2() = default;

    TextRenderer2::TextRenderer2(TextRenderer2&& rhs) noexcept
        : data_(std::move(rhs.data_))
    {
    }

    TextRenderer2& TextRenderer2::operator=(TextRenderer2&& rhs) noexcept
    {
        data_ = std::move(rhs.data_);
        return *this;
    }

    HorizontalAnchor TextRenderer2::horizontal_anchor() const
    {
        return data_->horizontal_anchor_;
    }

    void TextRenderer2::set_horizontal_anchor(HorizontalAnchor anchor)
    {
        data_->horizontal_anchor_ = anchor;
    }

    VerticalAnchor TextRenderer2::vertical_anchor() const
    {
        return data_->vertical_anchor_;
    }

    void TextRenderer2::set_vertical_anchor(VerticalAnchor alignment)
    {
        data_->vertical_anchor_ = alignment;
    }

    void TextRenderer2::add_text(const std::shared_ptr<TextItem>& item, TextPosition position)
    {

    }

    void TextRenderer2::remove_text(const std::shared_ptr<TextItem>& item)
    {
    }

    void TextRenderer2::refresh(const std::shared_ptr<TextItem>& item)
    {
    }

    void TextRenderer2::prepare(const Camera& camera)
    {
    }

    void TextRenderer2::render(const Camera& camera) const
    {
    }
} // Tungsten
