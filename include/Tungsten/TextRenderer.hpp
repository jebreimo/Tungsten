//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <unordered_map>
#include "Font.hpp"
#include "GlBuffer.hpp"
#include "GlTextures.hpp"

namespace Tungsten
{
    class TextRenderer
    {
    public:
        explicit TextRenderer(const Font& font);

        TextRenderer(const TextRenderer&) = delete;
        TextRenderer(TextRenderer&&) noexcept;
        ~TextRenderer();
        TextRenderer& operator=(const TextRenderer&) = delete;
        TextRenderer& operator=(TextRenderer&&) noexcept;

        void prepare_text(std::string_view text);

        void draw_text(const Xyz::Vector2F& pos,
                       const Xyz::Vector2F& screen_size) const;

        [[nodiscard]] Xyz::Vector2F get_size() const;

        [[nodiscard]] float line_gap() const;

        void set_line_gap(float line_gap);

        [[nodiscard]] const Yimage::Rgba8& color() const;

        void set_color(const Yimage::Rgba8& color);

    private:
        void initialize(std::string_view text);

        struct Data;
        std::unique_ptr<Data> data_;
        Tungsten::TextureHandle texture_;
        const Font* font_;
        Yimage::Rgba8 color_;
        Xyz::RectangleF text_rect_;
        float line_gap_ = 0.1;
    };

    [[nodiscard]] Xyz::Vector2F
    get_size(std::string_view text, const Font& font,
             float line_separator = 0.1);
}
