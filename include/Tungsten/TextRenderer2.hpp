//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>

#include "Renderable.hpp"
#include "TextItem.hpp"
#include "TextStyle.hpp"

namespace Tungsten
{
    struct RelativePosition
    {
        Xyz::Vector2F pos;
    };

    struct AbsolutePosition
    {
        Xyz::Vector2F pos;
    };

    using TextPosition = std::variant<RelativePosition, AbsolutePosition>;

    class TextRenderer2 : public Renderable
    {
    public:
        explicit TextRenderer2(Xyz::Vector2F position = {});

        ~TextRenderer2() override;

        TextRenderer2(const TextRenderer2&) = delete;

        TextRenderer2(TextRenderer2&&) noexcept;

        TextRenderer2& operator=(const TextRenderer2&) = delete;

        TextRenderer2& operator=(TextRenderer2&&) noexcept;

        [[nodiscard]] HorizontalAnchor horizontal_anchor() const;

        void set_horizontal_anchor(HorizontalAnchor anchor);

        [[nodiscard]] VerticalAnchor vertical_anchor() const;

        void set_vertical_anchor(VerticalAnchor alignment);

        void add_text(const std::shared_ptr<TextItem>& item, TextPosition position);

        void remove_text(const std::shared_ptr<TextItem>& item);

        void refresh(const std::shared_ptr<TextItem>& item);

        void prepare(const Camera& camera) override;

        void render(const Camera& camera) const override;

    private:
        struct Data;
        std::unique_ptr<Data> data_;
    };
} // Tungsten
