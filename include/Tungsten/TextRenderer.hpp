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

    struct TextRenderItem;
    struct FontRenderData;

    class TextRenderer : public Renderable
    {
    public:
        explicit TextRenderer();

        ~TextRenderer() override;

        TextRenderer(const TextRenderer&) = delete;

        TextRenderer(TextRenderer&&) noexcept;

        TextRenderer& operator=(const TextRenderer&) = delete;

        TextRenderer& operator=(TextRenderer&&) noexcept;

        size_t add_text_item(std::unique_ptr<TextItem> item);

        std::unique_ptr<TextItem> remove_text_item(size_t id);

        void clear_text_items();

        const TextItem* get_text_item(size_t id) const;

        TextItem* get_text_item(size_t id);

        void prepare(const Camera& camera) override;

        void render(const Camera& camera) const override;
    private:
        std::unique_ptr<TextRenderItem>
        make_render_data(const TextItem& item);

        static std::unique_ptr<FontRenderData>
        make_font_data(const std::shared_ptr<Font>& font);

        struct Data;
        std::unique_ptr<Data> data_;
    };
} // Tungsten
