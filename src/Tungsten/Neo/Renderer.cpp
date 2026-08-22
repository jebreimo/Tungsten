//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/Renderer.hpp"

#include <algorithm>
#include <cstring>

#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "UboBindings.hpp"

namespace Tungsten
{
    namespace
    {
        // The CPU-side image of the shaders' PerFrame block (std140,
        // binding 0) — a field-for-field match with the block declared in
        // BlinnPhong-*.glsl. Matrices are stored column-major, as GL expects;
        // each light is the leading 64 bytes of a LightData blob, which is
        // already std140-packed.
        struct PerFrameBlock
        {
            float view[16];
            float projection[16];
            float camera_pos[4]; // xyz = camera position, w = time
            float ambient_light[4]; // rgb
            float lights[MAX_LIGHTS][LIGHT_DATA_SIZE];
            int32_t light_count[4]; // x = number of active lights
        };

        static_assert(sizeof(PerFrameBlock) == 176 + MAX_LIGHTS * 64);
        // A light is copied straight across, so the two must agree exactly.
        static_assert(sizeof(PerFrameBlock::lights[0])
                      == sizeof(LightData{}.data()));

        void copy_column_major(const Xyz::Matrix4F& m, float* out)
        {
            for (size_t col = 0; col < 4; ++col)
            {
                for (size_t row = 0; row < 4; ++row)
                    *out++ = m[row, col];
            }
        }
    }

    Renderer::Renderer(ResourceManager& resources)
        : resources_(resources)
    {
        per_frame_ubo_ = generate_buffer();
        per_draw_ubo_ = generate_buffer();

        // Only binding 0 is set once and left alone; its buffer never changes,
        // so there is no repeated bind for the state cache to elide. Binding 1
        // is re-pointed at each material's own parameter buffer, and binding 2
        // at each item's slice of the packed per-draw buffer.
        bind_buffer_base(BufferTarget::UNIFORM, PER_FRAME_UBO_BINDING,
                         per_frame_ubo_.id());

        // Every bind_buffer_range offset must be a multiple of this, so the
        // per-draw blocks are spaced out to match. It is commonly 256 against
        // a 112-byte block; the padding is the price of one upload per frame.
        const auto alignment = size_t(get_uniform_buffer_offset_alignment());
        constexpr size_t block_size = RENDER_ITEM_DATA_SIZE * sizeof(float);
        per_draw_stride_ = (block_size + alignment - 1) / alignment * alignment;

        white_texture_ = generate_texture();
        bind_texture(TextureTarget::TEXTURE_2D, white_texture_.id());
        constexpr uint8_t white[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D, 0, {1, 1},
                             RGBA_TEXTURE, white);
        // The default min filter expects mipmaps; without this the lone
        // level-0 image would leave the texture incomplete.
        set_texture_parameters(TextureTarget::TEXTURE_2D,
                               {
                                   .mag_filter = SamplerMinMagFilter::NEAREST,
                                   .mip_filter = SamplerMipFilter::NONE
                               });
    }

    void Renderer::render(const RenderSnapshot& snapshot)
    {
        bind_per_frame(snapshot);
        // Forget the previous frame's material: its slot may have been
        // destroyed and reused between frames.
        current_material_ = {};

        // Both passes are sorted up front so every item's per-draw block can
        // go up in one upload; an item's index in sorted_ is its slot in the
        // per-draw buffer.
        sorted_.clear();
        append_sorted(snapshot.opaque_items);
        const size_t opaque_count = sorted_.size();
        append_sorted(snapshot.transparent_items);
        upload_per_draw_blocks();

        set_depth_test_enabled(true);
        set_blend_enabled(false);

        for (size_t i = 0; i < opaque_count; ++i)
            draw_item(*sorted_[i], i);

        if (opaque_count != sorted_.size())
        {
            set_blend_enabled(true);
            set_blend_function(BlendFunction::SRC_ALPHA,
                               BlendFunction::ONE_MINUS_SRC_ALPHA);
            for (size_t i = opaque_count; i < sorted_.size(); ++i)
                draw_item(*sorted_[i], i);
            set_blend_enabled(false);
        }
    }

    void Renderer::bind_per_frame(const RenderSnapshot& snapshot)
    {
        PerFrameBlock block = {};
        copy_column_major(snapshot.view_matrix, block.view);
        copy_column_major(snapshot.projection_matrix, block.projection);
        for (unsigned i = 0; i < 3; ++i)
        {
            block.camera_pos[i] = snapshot.camera_position[i];
            block.ambient_light[i] = snapshot.ambient_light[i];
        }
        block.camera_pos[3] = snapshot.time;

        const auto count = std::min<size_t>(snapshot.lights.size(), MAX_LIGHTS);
        for (size_t i = 0; i < count; ++i)
        {
            std::memcpy(block.lights[i], snapshot.lights[i].data().data(),
                        sizeof(block.lights[i]));
        }
        block.light_count[0] = static_cast<int32_t>(count);

        bind_buffer(BufferTarget::UNIFORM, per_frame_ubo_.id());
        set_buffer_data(BufferTarget::UNIFORM, sizeof(block), &block,
                        BufferUsage::DYNAMIC_DRAW);
    }

    void Renderer::append_sorted(const std::vector<RenderItem>& items)
    {
        const auto first = sorted_.size();
        sorted_.reserve(first + items.size());
        for (const RenderItem& item : items)
            sorted_.push_back(&item);
        // Only the newly appended run is sorted: the two passes are ordered
        // independently and must stay in the order they were appended, opaque
        // before transparent.
        std::sort(sorted_.begin() + ptrdiff_t(first), sorted_.end(),
                  [](const RenderItem* a, const RenderItem* b)
                  {
                      return a->sort_key() < b->sort_key();
                  });
    }

    void Renderer::upload_per_draw_blocks()
    {
        if (sorted_.empty())
            return;

        // One block per item, each starting at a offset the driver will accept
        // for glBindBufferRange. The padding between them is wasted, but it is
        // bounded by the alignment (typically 256 bytes against a 112-byte
        // block) and buys one upload per frame instead of one per draw.
        staging_.assign(sorted_.size() * per_draw_stride_, std::byte{});
        for (size_t i = 0; i < sorted_.size(); ++i)
        {
            std::memcpy(staging_.data() + i * per_draw_stride_,
                        sorted_[i]->data().data(),
                        RENDER_ITEM_DATA_SIZE * sizeof(float));
        }

        // Respecify rather than sub-update: handing the driver a fresh store
        // each frame lets it hand back memory the GPU is not still reading,
        // which is the cheap way to avoid a stall without fences.
        bind_buffer(BufferTarget::UNIFORM, per_draw_ubo_.id());
        set_buffer_data(BufferTarget::UNIFORM,
                        static_cast<ptrdiff_t>(staging_.size()),
                        staging_.data(), BufferUsage::DYNAMIC_DRAW);
    }

    void Renderer::draw_item(const RenderItem& item, size_t slot)
    {
        if (item.material() != current_material_)
            bind_material(item.material());

        const Mesh& mesh = resources_.get_mesh(item.mesh());
        state_.bind_vao(mesh.vao);

        // The block was uploaded with all the others; point binding 2 at this
        // item's slice of that buffer rather than rewriting it.
        bind_buffer_range(BufferTarget::UNIFORM, PER_DRAW_UBO_BINDING,
                          per_draw_ubo_.id(),
                          static_cast<ptrdiff_t>(slot * per_draw_stride_),
                          RENDER_ITEM_DATA_SIZE * sizeof(float));

        if (mesh.ebo.arena)
        {
            // The slice's offset is in index units, which is what
            // draw_elements takes; the indices are absolute (rebased at
            // upload, §3), so no base vertex is involved.
            draw_elements(mesh.primitive, mesh.index_type,
                          static_cast<int32_t>(mesh.ebo.offset),
                          static_cast<int32_t>(mesh.ebo.count));
        }
        else if (!mesh.streams.empty())
        {
            // Non-indexed: draw the first stream's range; its offset is the
            // base vertex.
            draw_array(mesh.primitive,
                       static_cast<int32_t>(mesh.streams[0].offset),
                       static_cast<int32_t>(mesh.streams[0].count));
        }
    }

    void Renderer::bind_material(MaterialRef ref)
    {
        const Material& material = resources_.get_material(ref);
        const ShaderProgram& shader = resources_.get_shader(material.shader);

        state_.use_program(shader.gl_handle.id());

        if (material.ubo)
        {
            // The parameters were uploaded when the material was created, so
            // switching materials only moves the binding point onto another
            // buffer. Consecutive items sharing a material bind nothing.
            state_.bind_material_ubo(material.ubo.id());
        }
        else if (shader.has_material_block)
        {
            // No buffer, but the shader reads the block — it would draw
            // against whichever material's buffer is still bound. Silently
            // wrong colours are worse than a thrown error.
            TUNGSTEN_THROW("Renderer: the material has no parameter_data, but"
                " its shader declares MaterialBlock.");
        }

        for (size_t i = 0; i < material.textures.size(); ++i)
        {
            state_.bind_texture(
                static_cast<int32_t>(i),
                resources_.get_texture(material.textures[i]).gl_handle.id());
        }

        // The program samples units [0, sampler_count) whether or not the
        // material fills them; the unfilled ones get the white texture.
        for (auto i = static_cast<uint32_t>(material.textures.size());
             i < shader.sampler_count; ++i)
        {
            state_.bind_texture(static_cast<int32_t>(i), white_texture_.id());
        }

        current_material_ = ref;
    }
} // Tungsten
