//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/TextRenderer.hpp"

#include <algorithm>
#include <unordered_set>

#include "FontUtilities.hpp"
#include "TextUtilities.hpp"
#include "TextRenderer/RenderTextShaderProgram.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/VertexArrayObjectBuilder.hpp"
#include "Tungsten/YimageGl.hpp"
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlProgram.hpp"
#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"
#include "Tungsten/Gl/GlTexture.hpp"

namespace Tungsten
{
    constexpr auto DIRTY_VERTEX_DATA_MASK = TextItem::DirtyFlags::TEXT
                                            | TextItem::DirtyFlags::FONT
                                            | TextItem::DirtyFlags::LINE_GAP
                                            | TextItem::DirtyFlags::HORIZONTAL_ALIGNMENT;

    struct TextRenderItem
    {
        VertexArrayObject vao;
        BufferHandle vbo;
        BufferHandle ebo;
        int32_t element_count = 0;
        Xyz::RectangleF rectangle;
        TextItem* item;
        FontRenderData* font_data = nullptr;
    };

    struct FontRenderData
    {
        TextureHandle texture;
        std::vector<TextRenderItem*> render_items;
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

        void replace_font(TextRenderItem& render_data, FontRenderData& font_data)
        {
            if (render_data.font_data)
            {
                std::erase_if(render_data.font_data->render_items,
                              [&](const auto& rd)
                              {
                                  return rd == &render_data;
                              });
            }
            render_data.font_data = &font_data;
            font_data.render_items.push_back(&render_data);
        }

        Xyz::Matrix3F make_transform(const TextItem& item,
                                     const Xyz::RectangleF& rect,
                                     const Viewport& viewport)
        {
            Xyz::Vector2F offset;
            switch (item.horizontal_anchor())
            {
            case HorizontalAnchor::LEFT:
                offset.x() = rect.origin.x();
                break;
            case HorizontalAnchor::CENTER:
                offset.x() = rect.origin.x() + rect.size.x() / 2;
                break;
            case HorizontalAnchor::RIGHT:
                offset.x() = rect.origin.x() + rect.size.x();
                break;
            }

            switch (item.vertical_anchor())
            {
            case VerticalAnchor::TOP:
                offset.y() = rect.origin.y() + rect.size.y();
                break;
            case VerticalAnchor::CENTER:
                offset.y() = rect.origin.y() + rect.size.y() / 2;
                break;
            case VerticalAnchor::BOTTOM:
                offset.y() = rect.origin.y();
                break;
            case VerticalAnchor::BASELINE:
                offset.y() = 0;
                break;
            }

            using namespace Xyz::affine;
            return scale2(2.f / viewport.size)
                   * translate2(item.position() - viewport.size / 2.f)
                   * rotate2(item.rotation())
                   * translate2(-offset);
        }
    }

    struct TextRenderer::Data
    {
        std::unordered_map<size_t, std::unique_ptr<TextItem>> text_entries_;
        std::unordered_map<size_t, std::unique_ptr<TextRenderItem>> text_data_;
        std::unordered_map<std::shared_ptr<Font>, std::unique_ptr<FontRenderData>> font_data_;
        Detail::RenderTextShaderProgram program;
        size_t next_id_ = 1;
    };

    TextRenderer::TextRenderer()
        : data_(std::make_unique<Data>())
    {
    }

    TextRenderer::~TextRenderer() = default;

    TextRenderer::TextRenderer(TextRenderer&& rhs) noexcept
        : data_(std::move(rhs.data_))
    {
    }

    TextRenderer& TextRenderer::operator=(TextRenderer&& rhs) noexcept
    {
        data_ = std::move(rhs.data_);
        return *this;
    }

    size_t TextRenderer::add_text_item(std::unique_ptr<TextItem> item)
    {
        if (!item->font())
            TUNGSTEN_THROW("Text item has no font assigned. (Text: " + item->text() + ")");
        const auto id = data_->next_id_++;
        auto [it, _] = data_->text_entries_.insert({id, std::move(item)});
        it->second->set_dirty_flags(TextItem::DirtyFlags::ALL);
        return id;
    }

    std::unique_ptr<TextItem> TextRenderer::remove_text_item(size_t id)
    {
        auto it = data_->text_entries_.find(id);
        if (it == data_->text_entries_.end())
            TUNGSTEN_THROW("No text item with id " + std::to_string(id) + " found.");
        auto item = std::move(it->second);
        data_->text_entries_.erase(it);
        auto render_it = data_->text_data_.find(id);
        if (render_it != data_->text_data_.end())
        {
            if (render_it->second->font_data)
            {
                std::erase_if(render_it->second->font_data->render_items,
                              [&](const auto& rd)
                              {
                                  return rd == render_it->second.get();
                              });
            }
            data_->text_data_.erase(render_it);
        }
        return item;
    }

    void TextRenderer::clear_text_items()
    {
        data_->text_entries_.clear();
        data_->text_data_.clear();
    }

    const TextItem* TextRenderer::get_text_item(size_t id) const
    {
        auto it = data_->text_entries_.find(id);
        if (it == data_->text_entries_.end())
            TUNGSTEN_THROW("No text item with id " + std::to_string(id) + " found.");
        return it->second.get();
    }

    TextItem* TextRenderer::get_text_item(size_t id)
    {
        auto it = data_->text_entries_.find(id);
        if (it == data_->text_entries_.end())
            TUNGSTEN_THROW("No text item with id " + std::to_string(id) + " found.");
        return it->second.get();
    }

    void TextRenderer::prepare(const Camera& camera)
    {
        std::vector<GlyphVertex> vertexes;
        std::vector<int32_t> indexes;
        for (auto& [id, item] : data_->text_entries_)
        {
            const auto dirty_flags = item->dirty_flags();
            if (!item->is_visible())
                continue;

            item->set_dirty_flags(TextItem::DirtyFlags::NONE);
            if ((dirty_flags & DIRTY_VERTEX_DATA_MASK) == TextItem::DirtyFlags::NONE)
                continue;

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

            auto rd_it = data_->text_data_.find(id);
            if (rd_it == data_->text_data_.end())
            {
                auto rd = std::make_unique<TextRenderItem>();
                rd->item = item.get();
                rd->vbo = generate_buffer();
                rd->ebo = generate_buffer();
                rd->vao = create_vertex_array(rd->vbo.id());
                rd->font_data = font_it->second.get();
                font_it->second->render_items.push_back(rd.get());
                rd_it = data_->text_data_.emplace(id, std::move(rd)).first;
            }
            else if (rd_it->second->font_data != font_it->second.get())
            {
                replace_font(*rd_it->second, *font_it->second);
            }

            vertexes.resize(0);
            indexes.resize(0);

            const auto text32 = utf8_to_utf32(item->text());
            const auto rect = add_vertexes(vertexes, indexes, *font,
                                           text32, item->line_gap(),
                                           item->horizontal_alignment());
            rd_it->second->element_count = int32_t(indexes.size());
            rd_it->second->rectangle = rect;
            bind_buffer(BufferTarget::ARRAY, rd_it->second->vbo.id());
            set_buffer_data(BufferTarget::ARRAY, std::span(vertexes),
                            BufferUsage::DYNAMIC_DRAW);
            bind_buffer(BufferTarget::ELEMENT_ARRAY, rd_it->second->ebo.id());
            set_buffer_data(BufferTarget::ELEMENT_ARRAY, std::span(indexes),
                            BufferUsage::DYNAMIC_DRAW);
        }
    }

    void TextRenderer::render(const Camera& camera) const
    {
        if (data_->text_entries_.empty())
            return;

        BlendRestorer blend_restorer;
        set_blend_enabled(true);
        set_blend_function(BlendFunction::SRC_ALPHA, BlendFunction::ONE_MINUS_SRC_ALPHA);

        data_->program.use();
        for (auto& [font, font_data] : data_->font_data_)
        {
            activate_texture_unit(0);
            bind_texture(TextureTarget::TEXTURE_2D, font_data->texture.id());
            for (auto& item : font_data->render_items)
            {
                if (!item->item->is_visible())
                    continue;
                const auto transform = make_transform(*item->item, item->rectangle, camera.viewport());
                data_->program.mvp_matrix.set(transform);
                data_->program.color.set(item->item->color());
                item->vao.bind();
                bind_buffer(BufferTarget::ARRAY, item->vbo.id());
                bind_buffer(BufferTarget::ELEMENT_ARRAY, item->ebo.id());
                draw_elements_32(TopologyType::TRIANGLES, 0, item->element_count);
            }
        }
    }

    std::unique_ptr<FontRenderData>
    TextRenderer::make_font_data(const std::shared_ptr<Font>& font)
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
