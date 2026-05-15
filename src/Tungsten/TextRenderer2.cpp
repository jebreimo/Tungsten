//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer2.hpp"

#include <algorithm>
#include <unordered_set>

#include "FontUtilities.hpp"
#include "TextUtilities.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/VertexArrayObjectBuilder.hpp"
#include "Tungsten/YimageGl.hpp"
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlProgram.hpp"
#include "Tungsten/Gl/GlTexture.hpp"

namespace Tungsten
{
    struct TextRenderData
    {
        VertexArrayObject vao;
        BufferHandle vbo;
        BufferHandle ebo;
        Xyz::Matrix3F transform;
        Xyz::Vector4F color;
        FontRenderData* font_data = nullptr;
    };

    struct FontRenderData
    {
        TextureHandle texture;
        std::vector<TextRenderData*> textures;
        ProgramHandle program;
    };

    namespace
    {
        using GlyphVertex = std::tuple<Xyz::Vector2F, Xyz::Vector2F>;

        VertexArrayObject create_vertex_array(uint32_t vertex_buffer)
        {
            return VertexArrayObjectBuilder()
                .bind_buffer(vertex_buffer)
                .add(0, VertexAttributeType::POSITION_2F)
                .add(1, VertexAttributeType::TEX_COORD_2F)
                .build();
        }

        void replace_font(TextRenderData& render_data, FontRenderData& font_data)
        {
            if (render_data.font_data)
            {
                std::erase_if(render_data.font_data->textures,
                              [&](const auto& rd)
                              {
                                  return rd == &render_data;
                              });
            }
            render_data.font_data = &font_data;
            font_data.textures.push_back(&render_data);
        }

        Xyz::Matrix3F make_transform(const TextItem& item, const Xyz::RectangleF& rect)
        {
            // Xyz::Vector2F offset;
            //     switch (item.style()->horizontal_anchor)
            //     {
            //     case HorizontalAnchor::LEFT:
            //         break;
            //     case HorizontalAnchor::CENTER:
            //         offset.x() = -rect.size.x() / 2;
            //         break;
            //     case HorizontalAnchor::RIGHT:
            //         offset.x() = -rect.size.x();
            //         break;
            //     }
            //
            //     switch (item.style()->vertical_anchor)
            //     {
            //     case VerticalAnchor::TOP:
            //         break;
            //     case VerticalAnchor::CENTER:
            //         offset.y() = rect.size.y() / 2;
            //         break;
            //     case VerticalAnchor::BOTTOM:
            //         offset.y() = rect.size.y();
            //         break;
            //     case VerticalAnchor::BASELINE:
            //         offset.y() = rect.origin.y();
            //         break;
            //     }
            auto t = Xyz::affine::translate2(item.position())
                     * Xyz::affine::rotate2(item.rotation())
                     * Xyz::affine::translate2(-rect.origin);
        }
    }

    struct TextRenderer2::Data
    {
        Xyz::Vector2F position_;
        Xyz::Vector2F size_;
        HorizontalAnchor horizontal_anchor_ = HorizontalAnchor::LEFT;
        VerticalAnchor vertical_anchor_ = VerticalAnchor::TOP;
        std::unordered_set<std::shared_ptr<TextItem>> text_entries_;
        std::unordered_set<std::shared_ptr<TextItem>> dirty_items_;
        std::unordered_map<std::shared_ptr<TextItem>, std::unique_ptr<TextRenderData>> text_data_;
        std::unordered_map<std::shared_ptr<Font>, std::unique_ptr<FontRenderData>> font_data_;
    };

    TextRenderer2::TextRenderer2(Xyz::Vector2F position)
        : data_(std::make_unique<Data>())
    {
    }

    TextRenderer2::~TextRenderer2() = default;

    TextRenderer2::TextRenderer2(TextRenderer2&& rhs) noexcept
        : data_(std::move(rhs.data_))
    {
    }

    TextRenderer2& TextRenderer2::operator=(TextRenderer2&& rhs) noexcept
    {
        data_ = std::move(rhs.data_);
        return *this;
    }

    HorizontalAnchor TextRenderer2::horizontal_anchor() const
    {
        return data_->horizontal_anchor_;
    }

    void TextRenderer2::set_horizontal_anchor(HorizontalAnchor anchor)
    {
        data_->horizontal_anchor_ = anchor;
    }

    VerticalAnchor TextRenderer2::vertical_anchor() const
    {
        return data_->vertical_anchor_;
    }

    void TextRenderer2::set_vertical_anchor(VerticalAnchor alignment)
    {
        data_->vertical_anchor_ = alignment;
    }

    void TextRenderer2::add_text(const std::shared_ptr<TextItem>& item)
    {
        if (!item->font())
            TUNGSTEN_THROW("Text item has no font assigned. (Text: " + item->text() + ")");
        if (data_->text_entries_.contains(item))
            TUNGSTEN_THROW("Text item is already added to the renderer.");
        data_->text_entries_.insert(item);
        data_->dirty_items_.insert(item);
    }

    void TextRenderer2::remove_text(const std::shared_ptr<TextItem>& item)
    {
        data_->text_entries_.erase(item);
        data_->dirty_items_.erase(item);
    }

    void TextRenderer2::refresh(const std::shared_ptr<TextItem>& item)
    {
        if (!data_->text_entries_.contains(item))
            TUNGSTEN_THROW("Cannot refresh text item that is not added to the renderer.");
        data_->dirty_items_.insert(item);
    }

    void TextRenderer2::prepare(const Camera& camera)
    {
        std::vector<GlyphVertex> vertexes;
        std::vector<int32_t> indexes;
        for (auto& item : data_->dirty_items_)
        {
            const auto& font = item->font();
            if (!font)
                TUNGSTEN_THROW("Text item has no font assigned. (Text: " + item->text() + ")");

            auto font_it = data_->font_data_.find(item->font());
            if (font_it == data_->font_data_.end())
            {
                auto font_data = make_font_data(item->font());
                font_it = data_->font_data_.emplace(item->font(),
                                                    std::move(font_data)).first;
            }

            auto rd_it = data_->text_data_.find(item);
            if (rd_it == data_->text_data_.end())
            {
                auto rd = std::make_unique<TextRenderData>();
                rd->vbo = generate_buffer(BufferTarget::ARRAY);
                rd->ebo = generate_buffer(BufferTarget::ELEMENT_ARRAY);
                rd->vao = create_vertex_array(rd->vbo.id());
                rd->font_data = font_it->second.get();
                font_it->second->textures.push_back(rd.get());
                rd_it = data_->text_data_.emplace(item, std::move(rd)).first;
            }
            else if (rd_it->second->font_data != font_it->second.get())
            {
                replace_font(*rd_it->second, *font_it->second);
                bind_buffer(BufferTarget::ARRAY, rd_it->second->vbo.id());
                bind_buffer(BufferTarget::ELEMENT_ARRAY, rd_it->second->ebo.id());
            }

            rd_it->second->color = item->color();

            vertexes.resize(0);
            indexes.resize(0);

            const auto text32 = utf8_to_utf32(item->text());
            auto rect = add_vertexes(vertexes, indexes, *font,
                                     text32, item->line_gap(),
                                     item->horizontal_alignment());
            rd_it->second->transform = Xyz::affine::translate2(item->position());
            set_buffer_data(BufferTarget::ARRAY, std::span(vertexes),
                            BufferUsage::DYNAMIC_DRAW);
            set_buffer_data(BufferTarget::ELEMENT_ARRAY, std::span(indexes),
                            BufferUsage::DYNAMIC_DRAW);
        }
    }

    std::unique_ptr<FontRenderData>
    TextRenderer2::make_font_data(const std::shared_ptr<Font>& font)
    {
        auto font_data = std::make_unique<FontRenderData>();
        font_data->texture = generate_texture();
        bind_texture(TextureTarget::TEXTURE_2D, font_data->texture.id());
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D,
                             0, get_size(font->image),
                             get_ogl_pixel_type(font->image.pixel_type()),
                             font->image.data());
        set_mag_filter(TextureTarget::TEXTURE_2D, TextureMagFilter::LINEAR);
        set_min_filter(TextureTarget::TEXTURE_2D, TextureMinFilter::LINEAR);
        set_wrap(TextureTarget::TEXTURE_2D, TextureWrapMode::CLAMP_TO_EDGE);

        return font_data;
    }
} // Tungsten
