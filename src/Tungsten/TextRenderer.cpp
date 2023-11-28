//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer.hpp"

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
        struct TextVertex
        {
            Xyz::Vector2F pos;
            Xyz::Vector2F texture;
        };

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
            std::u32string_view text,
            float line_separator)
        {
            const auto& glyphs = font.glyphs;
            if (glyphs.empty() || text.empty())
                return {};

            ArrayBuffer<TextVertex> buffer;

            unsigned lines = 0;
            float max_width = 0;
            Xyz::Vector2F pos;
            for (const auto c: text)
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
        std::u32string_view text,
        float line_separator)
    {
        const auto& glyphs = font.glyphs;
        if (glyphs.empty() || text.empty())
            return {};

        unsigned lines = 0;
        float max_width = 0;
        float width = 0;
        for (char32_t c : text)
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
        Tungsten::TextureHandle texture;
        Detail::RenderTextShaderProgram program;
    };

    TextRenderer::TextRenderer(const Font& font)
        : font_(&font)
    {}

    TextRenderer::TextRenderer(TextRenderer&&) noexcept = default;

    TextRenderer::~TextRenderer() = default;

    TextRenderer& TextRenderer::operator=(TextRenderer&&) noexcept = default;

    const Font& TextRenderer::font() const
    {
        return *font_;
    }

    void TextRenderer::draw(std::u32string_view text,
                            const Xyz::Vector2F& pos,
                            const Xyz::Vector2F& screen_size,
                            const TextProperties& properties)
    {
        if (!data_)
            initialize();

        auto [buffer, rect] = make_text_array_buffer(*font_, text,
                                                     properties.line_gap);
        set_buffers(data_->vertex_array, buffer);

        data_->program.color.set(to_vector(properties.color));
        auto [w, h] = screen_size;
        auto adjusted_pos = pos - rect.origin * 2.0f / Xyz::Vector2F(w, h);
        data_->program.mvp_matrix.set(Xyz::translate4(adjusted_pos[0], adjusted_pos[1], 0.f)
                                      * Xyz::scale4(2.0f / w, 2.0f / h, 1.0f));
        draw_triangle_elements_16(0, GLsizei(data_->vertex_array.indexes.size()));
    }

    void TextRenderer::initialize()
    {
        data_ = std::make_unique<Data>();

        data_->texture = generate_texture();
        bind_texture(GL_TEXTURE_2D, data_->texture);

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
            data_->program.position, 2, 0);
        enable_vertex_attribute(data_->program.position);
        data_->vertex_array.define_float_pointer(
            data_->program.texture_coord, 2, 2 * sizeof(float));
        enable_vertex_attribute(data_->program.texture_coord);
    }

    Xyz::Vector2F get_size(std::u32string_view text, const Font& font,
                           float line_gap)
    {
        return get_text_size(font, text, line_gap).size;
    }
}
