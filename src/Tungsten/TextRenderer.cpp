//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer.hpp"

#include <Yconvert/Convert.hpp>
#include "Tungsten/ArrayBufferBuilder.hpp"
#include "Tungsten/GlTextures.hpp"
#include "Tungsten/GlVertices.hpp"
#include "RenderTextShaderProgram.hpp"
#include "YimageGl.hpp"

#include "Debug.hpp"

namespace Tungsten
{
    namespace
    {
        struct TextVertex
        {
            Xyz::Vector2F pos;
            Xyz::Vector2F texture;
        };

        void add_rectangle(ArrayBufferBuilder<TextVertex> builder,
                           const Xyz::RectangleF& vertex_rect,
                           const Xyz::RectangleF& tex_rect)
        {
            JEB_SHOW(vertex_rect, tex_rect);
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
        ArrayBuffer<TextVertex>
        make_text_array_buffer(
            const std::unordered_map<char32_t, const Glyph*>& glyphs,
            std::string_view text)
        {
            if (glyphs.empty())
                return {};

            auto vertical_advance = glyphs.begin()->second->advance[1];

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
                if (!is_empty(glyph->glyph_rect))
                {
                    add_rectangle(buffer_builder,
                                  offset(glyph->glyph_rect, pos),
                                  glyph->tex_rect);
                }
                pos[0] += glyph->advance[0];
            }

            return buffer;
        }
    }

    TextRenderer::TextRenderer(const Font& font)
        : font_(&font),
          color_(0xFFFFFFFF)
    {
        for (auto& glyph : font.glyphs)
            glyphs_.insert({glyph.character, &glyph});
    }

    TextRenderer::TextRenderer(TextRenderer&&) noexcept = default;

    TextRenderer::~TextRenderer() = default;

    TextRenderer& TextRenderer::operator=(TextRenderer&&) noexcept = default;

    void TextRenderer::prepare_text(std::string_view text)
    {
        if (!program_)
        {
            initialize(text);
        }
        else
        {
            auto array_buffer = make_text_array_buffer(glyphs_, text);
            set_buffers(buffers_[0], buffers_[1], array_buffer);
        }
    }

    void TextRenderer::draw_text(const Xyz::Vector2F& pos,
                                 const Xyz::Vector2F& screen_size) const
    {
        program_->color.set(to_vector(color_));
        Xyz::Matrix4F mat =
            Xyz::translate4(pos[0], pos[1], 0.f)
            * Xyz::scale4(1.0f / screen_size[0], 1.0f / screen_size[1], 1.0f)
            ;
        program_->mvp_matrix.set(mat);
        draw_triangle_elements_16(0, GLsizei(count_));
    }

    const Yimage::Rgba8& TextRenderer::color() const
    {
        return color_;
    }

    void TextRenderer::set_color(const Yimage::Rgba8& color)
    {
        color_ = color;
    }

    void TextRenderer::initialize(std::string_view text)
    {
        vertex_array_ = generate_vertex_array();
        bind_vertex_array(vertex_array_);
        buffers_ = generate_buffers(2);
        auto array_buffer = make_text_array_buffer(glyphs_, text);
        count_ = array_buffer.indexes.size();
        set_buffers(buffers_[0], buffers_[1], array_buffer);

        texture_ = generate_texture();
        bind_texture(GL_TEXTURE_2D, texture_);

        set_texture_min_filter(GL_TEXTURE_2D, GL_LINEAR);
        set_texture_mag_filter(GL_TEXTURE_2D, GL_LINEAR);
        set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        auto [format, type] = get_ogl_pixel_type(font_->image.pixel_type());
        set_texture_image_2d(GL_TEXTURE_2D, 0, GL_RED,
                             GLsizei(font_->image.width()),
                             GLsizei(font_->image.height()),
                             format, type,
                             font_->image.data());

        program_ = std::make_unique<RenderTextShaderProgram>();

        use_program(program_->program);
        define_vertex_attribute_float_pointer(
            program_->position, 2, 4 * sizeof(float), 0);
        enable_vertex_attribute(program_->position);
        define_vertex_attribute_float_pointer(
            program_->texture_coord, 2, 4 * sizeof(float), 2 * sizeof(float));
        enable_vertex_attribute(program_->texture_coord);
    }
}
