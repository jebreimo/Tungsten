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
#include "Tungsten/YimageGl.hpp"
#include "Tungsten/VertexArray.hpp"
#include "Tungsten/details/RenderTextShaderProgram.hpp"

namespace Tungsten
{
    namespace
    {
        void add_rectangle(ArrayBuffer<TextVertex>& buffer,
                           const Xyz::RectangleF& vertex_rect,
                           const Xyz::RectangleF& tex_rect)
        {
            ArrayBufferBuilder<TextVertex> builder(buffer);
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
        std::pair<ArrayBuffer<TextVertex>, Xyz::RectangleF>
        make_text_array_buffer(
            const Font& font,
            std::string_view text,
            float line_separator)
        {
            const auto& glyphs = font.glyphs;
            if (glyphs.empty() || text.empty())
                return {};

            auto chars = Yconvert::convert_to<std::u32string>(
                text,
                Yconvert::Encoding::UTF_8,
                Yconvert::Encoding::UTF_32_NATIVE,
                Yconvert::ErrorPolicy::REPLACE
            );

            ArrayBuffer<TextVertex> buffer;

            unsigned lines = 0;
            float max_width = 0;
            Xyz::Vector2F pos;
            for (const auto c: chars)
            {
                if (c == '\n')
                {
                    ++lines;
                    max_width = std::max(max_width, pos[0]);
                    pos = {0, pos[1] - (1.f + line_separator) * font.max_glyph.size[1]};
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
                    add_rectangle(buffer,
                                  offset(glyph.glyph_rect, pos),
                                  glyph.tex_rect);
                }
                pos[0] += glyph.advance;
            }
            if (pos[0] > 0)
                ++lines;
            max_width = std::max(max_width, pos[0]);
            auto height =
                (float(lines) * (1 + line_separator) - line_separator) * font.max_glyph.size[1];
            auto y = font.max_glyph.origin[1]
                     - float(lines - 1) * (1 + line_separator) * font.max_glyph.size[1];
            Xyz::RectangleF rect({font.max_glyph.origin[0], y}, {max_width, height});
            return {buffer, rect};
        }
    }

    Xyz::RectangleF get_text_size(
        const Font& font,
        std::string_view text,
        float line_separator)
    {
        const auto& glyphs = font.glyphs;
        if (glyphs.empty() || text.empty())
            return {};

        auto chars = Yconvert::convert_to<std::u32string>(
            text,
            Yconvert::Encoding::UTF_8,
            Yconvert::Encoding::UTF_32_NATIVE,
            Yconvert::ErrorPolicy::REPLACE
        );

        unsigned lines = 0;
        float max_width = 0;
        float width = 0;
        for (char32_t c : chars)
        {
            if (c == '\n')
            {
                ++lines;
                max_width = std::max(max_width, width);
                width = 0;
            }

            auto it = glyphs.find(c);
            if (it == glyphs.end())
            {
                it = glyphs.find('?');
                if (it == glyphs.end())
                    continue;
            }

            auto& glyph = it->second;
            width += glyph.advance;
        }
        if (width > 0)
            ++lines;
        max_width = std::max(max_width, width);
        auto height = (float(lines) * (1 + line_separator) - line_separator) * font.max_glyph.size[1];
        auto y = font.max_glyph.origin[1] - float(lines - 1) * (1 + line_separator) * font.max_glyph.size[1];
        return {{font.max_glyph.origin[0], y}, {max_width, height}};
    }

    struct TextRenderer::Data
    {
        VertexArray<TextVertex> vertex_array;
        Detail::RenderTextShaderProgram program;
    };

    TextRenderer::TextRenderer(const Font& font)
        : font_(&font),
          color_(0xFFFFFFFF)
    {}

    TextRenderer::TextRenderer(TextRenderer&&) noexcept = default;

    TextRenderer::~TextRenderer() = default;

    TextRenderer& TextRenderer::operator=(TextRenderer&&) noexcept = default;

    const Font& TextRenderer::font() const
    {
        return *font_;
    }

    //void TextRenderer::prepare_text(std::string_view text)
    //{
    //    if (!data_)
    //    {
    //        initialize(text);
    //    }
    //    else
    //    {
    //        auto [buffer, rect] = make_text_array_buffer(*font_, text, line_gap_);
    //        set_buffers(data_->vertex_array, buffer);
    //        text_rect_ = rect;
    //    }
    //}

    //Xyz::Vector2F TextRenderer::get_size() const
    //{
    //    return text_rect_.size;
    //}

    void TextRenderer::draw_text(std::string_view text,
                                 const Xyz::Vector2F& pos,
                                 const Xyz::Vector2F& screen_size)
    {
        if (!data_)
            initialize(text);

        auto [buffer, rect] = make_text_array_buffer(*font_, text, line_gap_);
        set_buffers(data_->vertex_array, buffer);

        data_->program.color.set(to_vector(color_));
        auto [w, h] = screen_size;
        auto adjusted_pos = pos - rect.origin * 2.0f / Xyz::Vector2F(w, h);
        data_->program.mvp_matrix.set(Xyz::translate4(adjusted_pos[0], adjusted_pos[1], 0.f)
                                      * Xyz::scale4(2.0f / w, 2.0f / h, 1.0f));
        draw_triangle_elements_16(0, GLsizei(data_->vertex_array.indexes.size()));
    }

    float TextRenderer::line_gap() const
    {
        return line_gap_;
    }

    void TextRenderer::set_line_gap(float line_gap)
    {
        line_gap_ = line_gap;
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
        data_ = std::make_unique<Data>();
        //auto [buffer, rect] = make_text_array_buffer(*font_, text, line_gap_);
        //set_buffers(data_->vertex_array, buffer);
        //text_rect_ = rect;

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

        use_program(data_->program.program);
        data_->vertex_array.define_float_pointer(
            data_->program.position, 2, 4 * sizeof(float), 0);
        enable_vertex_attribute(data_->program.position);
        data_->vertex_array.define_float_pointer(
            data_->program.texture_coord, 2, 4 * sizeof(float), 2 * sizeof(float));
        enable_vertex_attribute(data_->program.texture_coord);
    }

    Xyz::Vector2F get_size(std::string_view text, const Font& font,
                           float line_gap)
    {
        return get_text_size(font, text, line_gap).size;
    }
}
