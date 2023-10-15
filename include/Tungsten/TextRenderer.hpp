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
#include "GlVertexArray.hpp"

namespace Tungsten
{
    class RenderTextShaderProgram;

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

        [[nodiscard]] float line_separator() const;

        void set_line_separator(float line_separator);

        [[nodiscard]] const Yimage::Rgba8& color() const;

        void set_color(const Yimage::Rgba8& color);

    private:
        void initialize(std::string_view text);

        std::vector<Tungsten::BufferHandle> buffers_;
        Tungsten::VertexArrayHandle vertex_array_;
        Tungsten::TextureHandle texture_;
        const Font* font_;
        Yimage::Rgba8 color_;
        std::unique_ptr<RenderTextShaderProgram> program_;
        size_t count_ = 0;
        Xyz::RectangleF text_rect_;
        float line_separator_ = 0.1;
    };

    [[nodiscard]] Xyz::Vector2F
    get_size(std::string_view text, const Font& font,
             float line_separator = 0.1);
}
