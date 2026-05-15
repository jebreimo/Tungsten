//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-08.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "TextStyleData.hpp"

namespace Tungsten
{
    class TextStyle
    {
    public:
        explicit TextStyle(std::shared_ptr<Font> font = {});

        explicit TextStyle(std::shared_ptr<TextStyle> parent);

        [[nodiscard]] const std::shared_ptr<TextStyle>& parent() const;

        /**
         *
         * @param parent Values that have not been assigned explicitly will
         *  be retrieved from @a parent by the `resolve_` methods.
         * @param keep_values Unless true, all values assigned that have been
         *  assigned to the current instance will be cleared, i.e. replaced
         *  by the values in @a parent.
         */
        TextStyle& set_parent(std::shared_ptr<TextStyle> parent, bool keep_values = true);

        void clear_all();

        [[nodiscard]] const std::shared_ptr<Font>& font() const;

        TextStyle& set_font(std::shared_ptr<Font> font);

        [[nodiscard]] bool has_font() const;

        TextStyle& clear_font();

        [[nodiscard]] const std::shared_ptr<Font>& resolve_font() const;

        [[nodiscard]] float line_gap() const;

        TextStyle& set_line_gap(float gap);

        [[nodiscard]] bool has_line_gap() const;

        TextStyle& clear_line_gap();

        [[nodiscard]] float resolve_line_gap() const;

        [[nodiscard]] Xyz::Vector4F color() const;

        TextStyle& set_color(Xyz::Vector4F color);

        [[nodiscard]] bool has_color() const;

        TextStyle& clear_color();

        [[nodiscard]] Xyz::Vector4F resolve_color() const;

        [[nodiscard]] HorizontalAlignment horizontal_alignment() const;

        TextStyle& set_horizontal_alignment(HorizontalAlignment alignment);

        [[nodiscard]] bool has_horizontal_alignment() const;

        TextStyle& clear_horizontal_alignment();

        [[nodiscard]] HorizontalAlignment resolve_horizontal_alignment() const;

        [[nodiscard]] HorizontalAnchor horizontal_anchor() const;

        TextStyle& set_horizontal_anchor(HorizontalAnchor anchor);

        [[nodiscard]] bool has_horizontal_anchor() const;

        TextStyle& clear_horizontal_anchor();

        [[nodiscard]] HorizontalAnchor resolve_horizontal_anchor() const;

        [[nodiscard]] VerticalAnchor vertical_anchor() const;

        TextStyle& set_vertical_anchor(VerticalAnchor anchor);

        [[nodiscard]] bool has_vertical_anchor() const;

        TextStyle& clear_vertical_anchor();

        [[nodiscard]] VerticalAnchor resolve_vertical_anchor() const;

    private:
        TextStyleData data_;
        uint8_t assigned_values_ = 0;
        std::shared_ptr<TextStyle> parent_;
    };
} // Tungsten
