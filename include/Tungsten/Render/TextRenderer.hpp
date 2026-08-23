//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include "Camera.hpp"
#include "TextItem.hpp"

namespace Tungsten
{
    struct TextRenderItem;
    struct FontRenderData;

    class TextRenderer
    {
    public:
        explicit TextRenderer();

        ~TextRenderer();

        TextRenderer(const TextRenderer&) = delete;

        TextRenderer(TextRenderer&&) noexcept;

        TextRenderer& operator=(const TextRenderer&) = delete;

        TextRenderer& operator=(TextRenderer&&) noexcept;

        size_t add_text_item(std::unique_ptr<TextItem> item);

        std::unique_ptr<TextItem> remove_text_item(size_t id);

        void clear_text_items();

        const TextItem* get_text_item(size_t id) const;

        TextItem* get_text_item(size_t id);

        void prepare(const Camera& camera);

        void render(const Camera& camera) const;
    private:
        static std::unique_ptr<FontRenderData>
        make_font_data(const std::shared_ptr<Font>& font);

        struct Data;
        std::unique_ptr<Data> data_;
    };
} // Tungsten
