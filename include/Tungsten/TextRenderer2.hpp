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

    struct TextRenderData;
    struct FontRenderData;

    class TextRenderer2 : public Renderable
    {
    public:
        explicit TextRenderer2();

        ~TextRenderer2() override;

        TextRenderer2(const TextRenderer2&) = delete;

        TextRenderer2(TextRenderer2&&) noexcept;

        TextRenderer2& operator=(const TextRenderer2&) = delete;

        TextRenderer2& operator=(TextRenderer2&&) noexcept;

        void add_text(const std::shared_ptr<TextItem>& item);

        void remove_text(const std::shared_ptr<TextItem>& item);

        void refresh(const std::shared_ptr<TextItem>& item);

        void prepare(const Camera& camera) override;

        void render(const Camera& camera) const override;
    private:
        std::unique_ptr<TextRenderData>
        make_render_data(const std::shared_ptr<TextItem>& item);

        static std::unique_ptr<FontRenderData>
        make_font_data(const std::shared_ptr<Font>& font);

        struct Data;
        std::unique_ptr<Data> data_;
    };
} // Tungsten
