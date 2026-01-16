//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-08-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer.hpp"

#include "Yconvert/Convert.hpp"
#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/TextRenderer/RenderTextShaderProgram.hpp"
#include "Tungsten/VertexArrayDataBuilder.hpp"
#include "Tungsten/YimageGl.hpp"

namespace Tungsten
{
    namespace
    {
        struct TextVertex
        {
            Xyz::Vector2F pos;
            Xyz::Vector2F texture;
        };

        void add_rectangle(VertexArrayData<TextVertex>& buffer,
                           const Xyz::Rectangle2F& vertex_rect,
                           const Xyz::Rectangle2F& tex_rect)
        {
            VertexArrayDataBuilder<TextVertex> builder(buffer);
            builder.reserve_vertexes(4);
            builder.add_vertex({
                vertex_rect[0],
                tex_rect[0]
            });
            builder.add_vertex({
                vertex_rect[1],
                tex_rect[1]
            });
            builder.add_vertex({
                vertex_rect[3],
                tex_rect[3]
            });
            builder.add_vertex({
                vertex_rect[2],
                tex_rect[2]
            });
            builder.reserve_indexes(6);
            builder.add_indexes(0, 1, 2);
            builder.add_indexes(2, 1, 3);
        }

        [[nodiscard]]
        std::pair<VertexArrayData<TextVertex>, Xyz::Rectangle2F>
        make_text_array_buffer(
            const Font& font,
            std::u32string_view text,
            float line_separator)
        {
            const auto& glyphs = font.glyphs;
            if (glyphs.empty() || text.empty())
                return {};

            VertexArrayData<TextVertex> buffer;

            unsigned lines = 0;
            float max_width = 0;
            Xyz::Vector2F pos;
            for (const auto c : text)
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
                    auto glyph_rect = glyph.glyph_rect;
                    glyph_rect.placement.origin += pos;
                    add_rectangle(buffer,
                                  glyph_rect,
                                  glyph.tex_rect);
                }
                pos[0] += glyph.advance;
            }
            if (pos[0] > 0)
                ++lines;
            max_width = std::max(max_width, pos[0]);
            auto height =
                (float(lines) * (1 + line_separator) - line_separator) * font.max_glyph.size[1];
            auto y = font.max_glyph.placement.origin.y()
                     - float(lines - 1) * (1 + line_separator) * font.max_glyph.size[1];
            Xyz::Rectangle2F rect({{font.max_glyph.placement.origin.x(), y}}, {max_width, height});
            return {buffer, rect};
        }
    }

    Xyz::Rectangle2F get_text_size(
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
        auto height = (float(lines) * (1 + line_separator) - line_separator)
                      * font.max_glyph.size[1];
        auto y = font.max_glyph.placement.origin.y() - float(lines - 1) * (1 + line_separator)
                 * font.max_glyph.size[1];
        return {{{font.max_glyph.placement.origin.x(), y}}, {max_width, height}};
    }

    struct TextRenderer::Data
    {
        const Font* font = nullptr;
        TextureHandle texture;
        Detail::RenderTextShaderProgram program;
        VertexArrayObject vao;
        bool auto_blend = true;
        Yconvert::Converter converter;

        explicit Data(const Font& font)
            : font(&font),
              texture(generate_texture()),
              converter(Yconvert::Encoding::UTF_8,
                        Yconvert::Encoding::UTF_32_NATIVE)
        {
            converter.set_error_policy(Yconvert::ErrorPolicy::THROW);
            bind_texture(TextureTarget::TEXTURE_2D, texture);

            set_min_filter(TextureTarget::TEXTURE_2D, TextureMinFilter::LINEAR);
            set_mag_filter(TextureTarget::TEXTURE_2D, TextureMagFilter::LINEAR);
            set_wrap(TextureTarget::TEXTURE_2D, TextureWrapMode::CLAMP_TO_EDGE);

            set_texture_image_2d(TextureTarget2D::TEXTURE_2D, 0,
                                 image_size(),
                                 get_ogl_pixel_type(font.image.pixel_type()),
                                 font.image.data());

            program.use();
            vao = program.create_vao();
            program.texture.set(0);
        }

        [[nodiscard]] Size2I image_size() const
        {
            return {int32_t(font->image.width()), int32_t(font->image.height())};
        }
    };

    TextRenderer::TextRenderer(const Font& font)
        : data_(std::make_unique<Data>(font))
    {
    }

    TextRenderer::TextRenderer(TextRenderer&&) noexcept = default;

    TextRenderer::~TextRenderer() = default;

    TextRenderer& TextRenderer::operator=(TextRenderer&&) noexcept = default;

    const Font& TextRenderer::font() const
    {
        return *data_->font;
    }

    bool TextRenderer::auto_blend() const
    {
        return data_->auto_blend;
    }

    void TextRenderer::set_auto_blend(bool value)
    {
        data_->auto_blend = value;
    }

    Xyz::Vector2F TextRenderer::get_size(std::string_view text, float line_gap) const
    {
        return get_size(to_u32(text), line_gap);
    }

    Xyz::Vector2F TextRenderer::get_size(std::u8string_view text, float line_gap) const
    {
        return get_size(to_u32(text), line_gap);
    }

    Xyz::Vector2F
    TextRenderer::get_size(std::u32string_view text, float line_gap) const
    {
        auto rect = get_text_size(*data_->font, text, line_gap);
        return {rect.size[0], rect.size[1]};
    }

    void TextRenderer::draw(std::string_view text,
                            const Xyz::Vector2F& pos,
                            const Xyz::Vector2F& screen_size,
                            const TextProperties& properties) const
    {
        draw(to_u32(text), pos, screen_size, properties);
    }

    void TextRenderer::draw(std::u8string_view text,
                            const Xyz::Vector2F& pos,
                            const Xyz::Vector2F& screen_size,
                            const TextProperties& properties) const
    {
        draw(to_u32(text), pos, screen_size, properties);
    }

    void TextRenderer::draw(std::u32string_view text,
                            const Xyz::Vector2F& pos,
                            const Xyz::Vector2F& screen_size,
                            const TextProperties& properties) const
    {
        const bool default_blend = is_blend_enabled();
        if (data_->auto_blend)
        {
            if (!default_blend)
                set_blend_enabled(true);
            set_blend_function(BlendFunction::SRC_ALPHA, BlendFunction::ONE_MINUS_SRC_ALPHA);
        }

        data_->program.use();

        activate_texture_unit(0);
        bind_texture(TextureTarget::TEXTURE_2D, data_->texture);

        auto [buffer, rect] = make_text_array_buffer(*data_->font, text,
                                                     properties.line_gap);
        data_->vao.set_data<TextVertex>(buffer.vertices, buffer.indices);

        data_->program.color.set(to_vector(properties.color));
        auto adjusted_pos = pos - rect.placement.origin * 2.0f / screen_size;
        auto [scale_x, scale_y] = 2.0f / screen_size;
        using namespace Xyz::affine;
        data_->program.mvp_matrix.set(translate3(adjusted_pos[0], adjusted_pos[1], 0.f)
                                      * scale3(scale_x, scale_y, 1.0f));
        draw_triangle_elements_16(0, data_->vao.element_count);

        set_blend_enabled(default_blend);
    }

    Size2I TextRenderer::image_size() const
    {
        return data_->image_size();
    }

    template <typename CharT>
    std::u32string TextRenderer::to_u32(std::basic_string_view<CharT> str) const
    {
        return Yconvert::convert_to<std::u32string>(str, data_->converter);
    }
}
