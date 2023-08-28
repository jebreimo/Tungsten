//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer.hpp"

#include <Yconvert/Convert.hpp>
#include <Yimage/Yimage.hpp>
#include "Tungsten/ArrayBufferBuilder.hpp"
#include "Tungsten/GlTextures.hpp"
#include "YimageGl.hpp"

namespace Tungsten
{
    namespace
    {
        struct TextVertex
        {
            Xyz::Vector2F pos;
            Xyz::Vector2F texture;
        };

        void add_rectangle(Tungsten::ArrayBufferBuilder<TextVertex> builder,
                           const Xyz::RectangleF& vertex_rect,
                           const Xyz::RectangleF& tex_rect)
        {
            builder.reserve_vertexes(4);
            builder.add_vertex({get_bottom_left(vertex_rect),
                                get_bottom_left(tex_rect)});
            builder.add_vertex({get_bottom_right(vertex_rect),
                                get_bottom_right(tex_rect)});
            builder.add_vertex({get_top_left(vertex_rect),
                                get_top_left(tex_rect)});
            builder.add_vertex({get_top_right(vertex_rect),
                                get_top_right(tex_rect)});
            builder.reserve_indexes(6);
            builder.add_indexes(0, 1, 2);
            builder.add_indexes(2, 1, 3);
        }

        [[nodiscard]]
        Tungsten::ArrayBuffer<TextVertex>
        make_text_array_buffer(
            const std::unordered_map<char32_t, Glyph>& glyphs,
            std::string_view text)
        {
            if (glyphs.empty())
                return {};

            auto vertical_advance = glyphs.begin()->second.advance[1];

            auto chars = Yconvert::convert_to<std::u32string>(
                text,
                Yconvert::Encoding::UTF_8,
                Yconvert::Encoding::UTF_32_NATIVE,
                Yconvert::ErrorPolicy::REPLACE
            );

            ArrayBuffer<TextVertex> buffer;
            ArrayBufferBuilder buffer_builder(buffer);

            Xyz::Vector2F pos;
            for (const auto c: chars)
            {
                if (c == '\n')
                {
                    pos = {0, pos[1] + vertical_advance};
                    continue;
                }

                auto it = glyphs.find(c);
                if (it == glyphs.end())
                {
                    if (it = glyphs.find('?'); it == glyphs.end())
                        continue;
                }

                auto& glyph = it->second;
                if (!is_empty(glyph.glyph_rect))
                {
                    add_rectangle(buffer_builder,
                                  offset(glyph.glyph_rect, pos),
                                  offset(glyph.tex_rect, pos));
                }
                pos[0] += glyph.advance[0];
            }

            return buffer;
        }
    }

    TextRenderer::TextRenderer(const Font& font)
        : font_name_(font.family_name),
          font_size_(font.size)
    {
        for (auto& glyph : font.glyphs)
            glyphs_.insert({glyph.character, glyph});
        image_ = Yimage::read_image(font.image_buffer, font.image_buffer_size);
    }

    void TextRenderer::prepare_text(std::string_view text)
    {
        auto array_buffer = make_text_array_buffer(glyphs_, text);

        vertex_array_ = Tungsten::generate_vertex_array();
        Tungsten::bind_vertex_array(vertex_array_);
        buffers_ = generate_buffers(2);
        set_buffers(buffers_[0], buffers_[1], array_buffer);

        texture_ = Tungsten::generate_texture();
        Tungsten::bind_texture(GL_TEXTURE_2D, texture_);

        Tungsten::set_texture_min_filter(GL_TEXTURE_2D, GL_LINEAR);
        Tungsten::set_texture_mag_filter(GL_TEXTURE_2D, GL_LINEAR);
        Tungsten::set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        Tungsten::set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        auto [format, type] = get_ogl_pixel_type(image_.pixel_type());
        set_texture_image_2d(GL_PROXY_TEXTURE_2D, 0, GL_RED,
                             GLsizei(image_.width()),
                             GLsizei(image_.height()),
                             format, type,
                             image_.data());
    }

    void TextRenderer::draw_text()
    {

    }
}
