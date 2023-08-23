//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlBuffer.hpp"
#include "GlTextures.hpp"
#include "GlVertexArray.hpp"
#include "Font.hpp"

namespace Tungsten
{
    class TextRenderer
    {
    public:
        explicit TextRenderer(const Font& font);

        void prepare_text(std::string_view text);

        void draw_text();

    private:
        std::vector<Tungsten::BufferHandle> buffers_;
        Tungsten::VertexArrayHandle vertex_array_;
        Tungsten::TextureHandle texture_;
        std::unordered_map<char32_t, Glyph> glyphs_;
        Yimage::Image image_;
        std::string font_name_;
        unsigned font_size_;
    };
}
