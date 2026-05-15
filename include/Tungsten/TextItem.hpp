//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "TextStyle.hpp"

namespace Tungsten
{
    using TextId = uint32_t;

    class TextRenderer2;

    class TextItem
    {
    public:
        TextItem() = default;

        explicit TextItem(std::string text,
                          std::shared_ptr<TextStyle> style);

        [[nodiscard]] const std::string& text() const;

        void set_text(std::string text);

        [[nodiscard]] const Xyz::Vector2F& position() const;

        void set_position(const Xyz::Vector2F& position);

        [[nodiscard]] float rotation() const;

        void set_rotation(float rotation);

        [[nodiscard]] const std::shared_ptr<TextStyle>& style() const;

        void set_style(std::shared_ptr<TextStyle> style);

        [[nodiscard]] Xyz::Vector2F size() const;

    private:
        std::string text_;
        Xyz::Vector2F position_;
        float rotation_;
        std::shared_ptr<TextStyle> style_;
    };
}
