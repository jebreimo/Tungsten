//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "TextStyleData.hpp"
#include "../Detail/GenericBitmaskOperators.hpp"

namespace Tungsten
{
    using TextId = uint32_t;

    class TextRenderer;

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

        [[nodiscard]] const Xyz::Vector4F& color() const;

        void set_color(const Xyz::Vector4F& color);

        [[nodiscard]] HorizontalAlignment horizontal_alignment() const;

        void set_horizontal_alignment(HorizontalAlignment alignment);

        [[nodiscard]] HorizontalAnchor horizontal_anchor() const;

        void set_horizontal_anchor(HorizontalAnchor anchor);

        [[nodiscard]] VerticalAnchor vertical_anchor() const;

        void set_vertical_anchor(VerticalAnchor anchor);

        [[nodiscard]] bool is_visible() const;

        void set_visible(bool visible);

        [[nodiscard]] Xyz::Vector2F calc_size() const;

        enum class DirtyFlags
        {
            NONE = 0,
            TEXT = 1 << 0,
            POSITION = 1 << 1,
            ROTATION = 1 << 2,
            FONT = 1 << 3,
            LINE_GAP = 1 << 4,
            COLOR = 1 << 5,
            HORIZONTAL_ALIGNMENT = 1 << 6,
            HORIZONTAL_ANCHOR = 1 << 7,
            VERTICAL_ANCHOR = 1 << 8,
            VISIBLE = 1 << 9,
            ALL = 0x3FF
        };

        [[nodiscard]] DirtyFlags dirty_flags() const;

        void set_dirty_flags(DirtyFlags flags);

    private:
        std::string text_;
        Xyz::Vector2F position_;
        float rotation_ = 0.f;
        std::shared_ptr<Font> font_;
        float line_gap_ = 0.1f;
        Xyz::Vector4F color_ = {0.f, 0.f, 0.f, 1.f};
        HorizontalAlignment horizontal_alignment_ = HorizontalAlignment::LEFT;
        HorizontalAnchor horizontal_anchor_ = HorizontalAnchor::LEFT;
        VerticalAnchor vertical_anchor_ = VerticalAnchor::TOP;
        bool visible_ = true;
        DirtyFlags dirty_flags_ = DirtyFlags::ALL;
    };

    TUNGSTEN_ENABLE_BITMASK_OPERATORS(TextItem::DirtyFlags);
}
