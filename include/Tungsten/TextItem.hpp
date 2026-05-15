//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "TextStyleData.hpp"

namespace Tungsten
{
    using TextId = uint32_t;

    class TextRenderer2;

    class TextItem
    {
    public:
        TextItem() = default;

        TextItem(std::string text, std::shared_ptr<Font> style);

        TextItem(std::string text, const TextStyleData& style);

        [[nodiscard]] const std::string& text() const;

        void set_text(std::string text);

        [[nodiscard]] const Xyz::Vector2F& position() const;

        void set_position(const Xyz::Vector2F& position);

        [[nodiscard]] float rotation() const;

        void set_rotation(float rotation);

        [[nodiscard]] const std::shared_ptr<Font>& font() const;

        void set_font(std::shared_ptr<Font> font);

        [[nodiscard]] float line_gap() const;

        void set_line_gap(float gap);

        [[nodiscard]] Xyz::Vector4F color() const;

        void set_color(Xyz::Vector4F color);

        [[nodiscard]] HorizontalAlignment horizontal_alignment() const;

        void set_horizontal_alignment(HorizontalAlignment alignment);

        [[nodiscard]] HorizontalAnchor horizontal_anchor() const;

        void set_horizontal_anchor(HorizontalAnchor anchor);

        [[nodiscard]] VerticalAnchor vertical_anchor() const;

        void set_vertical_anchor(VerticalAnchor anchor);

        [[nodiscard]] Xyz::Vector2F caclulate_size() const;

    private:
        std::string text_;
        Xyz::Vector2F position_;
        float rotation_ = 0.f;
        std::shared_ptr<Font> font_;
        float line_gap_ = 0.5;
        Xyz::Vector4F color_ = {0.f, 0.f, 0.f, 1.f};
        HorizontalAlignment horizontal_alignment_ = HorizontalAlignment::LEFT;
        HorizontalAnchor horizontal_anchor_ = HorizontalAnchor::LEFT;
        VerticalAnchor vertical_anchor_ = VerticalAnchor::TOP;
    };
}
