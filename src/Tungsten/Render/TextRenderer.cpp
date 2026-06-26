//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/TextRenderer.hpp"

#include <algorithm>
#include <bit>

#include "../Detail/BuddyAllocator.hpp"
#include "FontUtilities.hpp"
#include "TextUtilities.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Render/VertexArrayObjectBuilder.hpp"
#include "Tungsten/YimageGl.hpp"
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "TextRenderer/RenderTextShaderProgram.hpp"

namespace Tungsten
{
    constexpr auto DIRTY_VERTEX_DATA_MASK = TextItem::DirtyFlags::TEXT
                                            | TextItem::DirtyFlags::FONT
                                            | TextItem::DirtyFlags::LINE_GAP
                                            | TextItem::DirtyFlags::HORIZONTAL_ALIGNMENT;

    constexpr size_t INITIAL_VERTEX_CAPACITY = 4096; // 1024 quads
    constexpr size_t INITIAL_INDEX_CAPACITY = 8192; // next pow2 above 1024*6=6144

    // Sentinel: this render item has no GPU allocation yet.
    constexpr size_t UNALLOCATED = SIZE_MAX;

    struct TextRenderItem
    {
        size_t vertex_offset = UNALLOCATED;
        size_t vertex_alloc_size = 0; // power-of-2 block size in the buddy allocator
        size_t index_offset = UNALLOCATED;
        size_t index_alloc_size = 0;
        int32_t element_count = 0;
        Xyz::RectangleF rectangle;
        TextItem* item = nullptr;
        FontRenderData* font_data = nullptr;
    };

    struct FontRenderData
    {
        TextureHandle texture;
        std::vector<TextRenderItem*> render_items;
    };

    // Holds the GPU buffers and buddy allocators shared across all fonts.
    // Defined at namespace scope so anonymous-namespace helpers can reference it.
    struct TextGlBuffers
    {
        VertexArrayObject vao;
        BufferHandle vbo;
        BufferHandle ebo;
        BuddyAllocator vertex_alloc{INITIAL_VERTEX_CAPACITY};
        BuddyAllocator index_alloc{INITIAL_INDEX_CAPACITY};
    };

    struct TextRenderer::Data
    {
        std::unordered_map<size_t, std::unique_ptr<TextItem>> text_entries_;
        std::unordered_map<size_t, std::unique_ptr<TextRenderItem>> text_data_;
        std::unordered_map<std::shared_ptr<Font>, std::unique_ptr<FontRenderData>> font_data_;
        Detail::RenderTextShaderProgram program;
        size_t next_id_ = 1;
        TextGlBuffers gl;
    };

    namespace
    {
        using TextDataMap = std::unordered_map<size_t, std::unique_ptr<TextRenderItem>>;

        VertexArrayObject create_vertex_array(uint32_t vbo_id)
        {
            return VertexArrayObjectBuilder()
                .bind_buffer(vbo_id)
                .add(0, VertexAttributeType::POSITION_2F)
                .add(1, VertexAttributeType::TEX_COORD_2F)
                .build();
        }

        void init_gl_buffers(TextGlBuffers& gl)
        {
            gl.vbo = generate_buffer(ptrdiff_t(INITIAL_VERTEX_CAPACITY * sizeof(GlyphVertex)),
                                     BufferUsage::DYNAMIC_DRAW,
                                     BufferTarget::ARRAY);
            gl.ebo = generate_buffer(ptrdiff_t(INITIAL_INDEX_CAPACITY * sizeof(int32_t)),
                                     BufferUsage::DYNAMIC_DRAW,
                                     BufferTarget::ELEMENT_ARRAY);
            gl.vao = create_vertex_array(gl.vbo.id());
        }

        void replace_font(TextRenderItem& rd, FontRenderData& new_font,
                          BuddyAllocator& vertex_alloc, BuddyAllocator& index_alloc)
        {
            if (rd.font_data)
            {
                if (rd.vertex_offset != UNALLOCATED)
                {
                    vertex_alloc.free(rd.vertex_offset);
                    index_alloc.free(rd.index_offset);
                    rd.vertex_offset = UNALLOCATED;
                    rd.index_offset = UNALLOCATED;
                }
                std::erase(rd.font_data->render_items, &rd);
            }
            rd.font_data = &new_font;
            new_font.render_items.push_back(&rd);
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

        // Doubles the vertex buffer, preserving existing content, and rebuilds
        // the buddy allocator by re-claiming all live allocations.
        // Uses explicit COPY_READ/COPY_WRITE bindings to avoid the broken
        // resize_buffer path that calls copy_buffer with COPY_WRITE unbound.
        void grow_vertex_buffer(TextGlBuffers& gl, const TextDataMap& text_data)
        {
            const size_t old_cap = gl.vertex_alloc.capacity();
            const size_t new_cap = old_cap * 2;
            const auto new_bytes = ptrdiff_t(new_cap * sizeof(GlyphVertex));
            resize_buffer(gl.vbo.id(), new_bytes);

            BuddyAllocator new_alloc(new_cap);
            for (const auto& [id, rd] : text_data)
            {
                if (rd->vertex_offset != UNALLOCATED)
                    new_alloc.claim(rd->vertex_offset, rd->vertex_alloc_size);
            }
            gl.vertex_alloc = std::move(new_alloc);
        }

        // Doubles the index buffer, preserving existing content, and rebuilds
        // the buddy allocator.
        void grow_index_buffer(TextGlBuffers& gl, const TextDataMap& text_data)
        {
            const size_t old_cap = gl.index_alloc.capacity();
            const size_t new_cap = old_cap * 2;
            const auto new_bytes = ptrdiff_t(new_cap * sizeof(int32_t));

            resize_buffer(gl.ebo.id(), new_bytes);

            BuddyAllocator new_alloc(new_cap);
            for (const auto& [id, rd] : text_data)
            {
                if (rd->index_offset != UNALLOCATED)
                    new_alloc.claim(rd->index_offset, rd->index_alloc_size);
            }
            gl.index_alloc = std::move(new_alloc);
        }
    }

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
        const auto it = data_->text_entries_.find(id);
        if (it == data_->text_entries_.end())
            TUNGSTEN_THROW("No text item with id " + std::to_string(id) + " found.");
        auto item = std::move(it->second);
        data_->text_entries_.erase(it);

        auto rd_it = data_->text_data_.find(id);
        if (rd_it != data_->text_data_.end())
        {
            TextRenderItem* rd = rd_it->second.get();
            if (rd->font_data)
            {
                if (rd->vertex_offset != UNALLOCATED)
                {
                    data_->gl.vertex_alloc.free(rd->vertex_offset);
                    data_->gl.index_alloc.free(rd->index_offset);
                }
                std::erase(rd->font_data->render_items, rd);
            }
            data_->text_data_.erase(rd_it);
        }
        return item;
    }

    void TextRenderer::clear_text_items()
    {
        data_->text_entries_.clear();
        data_->text_data_.clear();
        data_->font_data_.clear();
        // Reset allocators to current capacity (all free); GPU buffers are reused as-is.
        data_->gl.vertex_alloc = BuddyAllocator(data_->gl.vertex_alloc.capacity());
        data_->gl.index_alloc = BuddyAllocator(data_->gl.index_alloc.capacity());
    }

    const TextItem* TextRenderer::get_text_item(size_t id) const
    {
        const auto it = data_->text_entries_.find(id);
        if (it == data_->text_entries_.end())
            TUNGSTEN_THROW("No text item with id " + std::to_string(id) + " found.");
        return it->second.get();
    }

    TextItem* TextRenderer::get_text_item(size_t id)
    {
        const auto it = data_->text_entries_.find(id);
        if (it == data_->text_entries_.end())
            TUNGSTEN_THROW("No text item with id " + std::to_string(id) + " found.");
        return it->second.get();
    }

    void TextRenderer::prepare(const Camera& camera)
    {
        // --- Phase 1: compute vertex/index data for all dirty visible items ---

        struct PendingItem
        {
            TextRenderItem* rd = nullptr;
            std::vector<GlyphVertex> vertexes;
            std::vector<int32_t> indexes;
            Xyz::RectangleF rectangle;
        };

        std::vector<PendingItem> pending;

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
                TUNGSTEN_THROW("Text item has no font. (Text: " + item->text() + ")");

            // Get or create FontRenderData for this font.
            auto font_it = data_->font_data_.find(font);
            if (font_it == data_->font_data_.end())
            {
                if (!data_->gl.vbo.id())
                    init_gl_buffers(data_->gl);
                auto fd = make_font_data(font);
                font_it = data_->font_data_.emplace(font, std::move(fd)).first;
            }
            FontRenderData* fd = font_it->second.get();

            // Get or create TextRenderItem for this entry.
            auto rd_it = data_->text_data_.find(id);
            if (rd_it == data_->text_data_.end())
            {
                auto rd = std::make_unique<TextRenderItem>();
                rd->item = item.get();
                rd->font_data = fd;
                fd->render_items.push_back(rd.get());
                rd_it = data_->text_data_.emplace(id, std::move(rd)).first;
            }
            else if (rd_it->second->font_data != fd)
            {
                replace_font(*rd_it->second, *fd,
                             data_->gl.vertex_alloc, data_->gl.index_alloc);
            }

            TextRenderItem* rd = rd_it->second.get();

            PendingItem pi;
            pi.rd = rd;
            const auto text32 = utf8_to_utf32(item->text());
            pi.rectangle = add_vertexes(pi.vertexes, pi.indexes, *font,
                                        text32, item->line_gap(),
                                        item->horizontal_alignment());
            pending.push_back(std::move(pi));
        }

        if (pending.empty())
            return;

        // --- Phase 2: free old allocations, grow if needed, upload ---

        // Release the GPU ranges that will be replaced.
        for (auto& pi : pending)
        {
            TextRenderItem* rd = pi.rd;
            if (rd->vertex_offset != UNALLOCATED)
            {
                data_->gl.vertex_alloc.free(rd->vertex_offset);
                data_->gl.index_alloc.free(rd->index_offset);
                rd->vertex_offset = UNALLOCATED;
                rd->index_offset = UNALLOCATED;
            }
        }

        // Allocate space for each item, growing buffers on demand.
        for (auto& pi : pending)
        {
            TextRenderItem* rd = pi.rd;

            if (pi.vertexes.empty())
            {
                rd->element_count = 0;
                rd->rectangle = {};
                continue;
            }

            std::optional<size_t> v_off;
            std::optional<size_t> i_off;

            while (true)
            {
                v_off = data_->gl.vertex_alloc.allocate(pi.vertexes.size());
                i_off = v_off
                            ? data_->gl.index_alloc.allocate(pi.indexes.size())
                            : std::nullopt;
                if (v_off && i_off)
                    break;

                // Release whichever succeeded before growing the right buffer.
                if (v_off && !i_off)
                {
                    data_->gl.vertex_alloc.free(*v_off);
                    grow_index_buffer(data_->gl, data_->text_data_);
                }
                else
                {
                    grow_vertex_buffer(data_->gl, data_->text_data_);
                }
            }

            rd->vertex_offset = *v_off;
            rd->vertex_alloc_size = std::bit_ceil(pi.vertexes.size());
            rd->index_offset = *i_off;
            rd->index_alloc_size = std::bit_ceil(pi.indexes.size());
            rd->element_count = int32_t(pi.indexes.size());
            rd->rectangle = pi.rectangle;

            // Upload vertex data into the shared VBO at the allocated slot.
            bind_buffer(BufferTarget::ARRAY, data_->gl.vbo.id());
            set_buffer_subdata(BufferTarget::ARRAY,
                               std::span(pi.vertexes),
                               ptrdiff_t(*v_off * sizeof(GlyphVertex)));

            // Adjust indices from item-local to global vertex positions,
            // then upload into the shared EBO at the allocated slot.
            for (auto& idx : pi.indexes)
                idx += int32_t(*v_off);
            bind_buffer(BufferTarget::ELEMENT_ARRAY, data_->gl.ebo.id());
            set_buffer_subdata(BufferTarget::ELEMENT_ARRAY,
                               std::span(pi.indexes),
                               ptrdiff_t(*i_off * sizeof(int32_t)));
        }
    }

    void TextRenderer::render(const Camera& camera) const
    {
        if (data_->font_data_.empty())
            return;

        BlendRestorer blend_restorer;
        set_blend_enabled(true);
        set_blend_function(BlendFunction::SRC_ALPHA, BlendFunction::ONE_MINUS_SRC_ALPHA);

        data_->program.use();
        data_->gl.vao.bind();
        bind_buffer(BufferTarget::ELEMENT_ARRAY, data_->gl.ebo.id());

        for (auto& [font, fd] : data_->font_data_)
        {
            if (fd->render_items.empty())
                continue;

            activate_texture_unit(0);
            bind_texture(TextureTarget::TEXTURE_2D, fd->texture.id());

            for (const TextRenderItem* rd : fd->render_items)
            {
                if (!rd->item->is_visible() || rd->element_count == 0)
                    continue;

                const auto transform = make_transform(*rd->item, rd->rectangle,
                                                      camera.viewport());
                data_->program.mvp_matrix.set(transform);
                data_->program.color.set(rd->item->color());

                // index_offset is in index units; draw_elements_32 converts to bytes.
                draw_elements_32(TopologyType::TRIANGLES,
                                 int32_t(rd->index_offset),
                                 rd->element_count);
            }
        }
    }

    std::unique_ptr<FontRenderData>
    TextRenderer::make_font_data(const std::shared_ptr<Font>& font)
    {
        auto fd = std::make_unique<FontRenderData>();

        fd->texture = generate_texture();
        bind_texture(TextureTarget::TEXTURE_2D, fd->texture.id());
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D,
                             0, get_size(font->image),
                             get_ogl_pixel_type(font->image.pixel_type()),
                             font->image.data());
        set_mag_filter(TextureTarget::TEXTURE_2D, TextureMagFilter::LINEAR);
        set_min_filter(TextureTarget::TEXTURE_2D, TextureMinFilter::LINEAR);
        set_wrap(TextureTarget::TEXTURE_2D, TextureWrapMode::CLAMP_TO_EDGE);

        return fd;
    }
} // Tungsten
