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
        per_material_ubo_ = generate_buffer();
        per_draw_ubo_ = generate_buffer();

        // All three binding points are set once here and never again; only the
        // buffers' contents change afterwards. That is why none of these goes
        // through the state cache — there is no repeated bind for it to elide.
        bind_buffer_base(BufferTarget::UNIFORM, PER_FRAME_UBO_BINDING,
                         per_frame_ubo_.id());
        bind_buffer_base(BufferTarget::UNIFORM, PER_DRAW_UBO_BINDING,
                         per_draw_ubo_.id());
        bind_buffer_base(BufferTarget::UNIFORM, PER_MATERIAL_UBO_BINDING,
                         per_material_ubo_.id());

        white_texture_ = generate_texture();
        bind_texture(TextureTarget::TEXTURE_2D, white_texture_.id());
        constexpr uint8_t white[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D, 0, {1, 1},
                             RGBA_TEXTURE, white);
        // The default min filter expects mipmaps; without this the lone
        // level-0 image would leave the texture incomplete.
        set_min_filter(TextureTarget::TEXTURE_2D, TextureMinFilter::NEAREST);
        set_mag_filter(TextureTarget::TEXTURE_2D, TextureMagFilter::NEAREST);
    }

    void Renderer::render(const RenderSnapshot& snapshot)
    {
        bind_per_frame(snapshot);
        // Forget the previous frame's material: its slot may have been
        // destroyed and reused between frames.
        current_material_ = {};

        set_depth_test_enabled(true);
        set_blend_enabled(false);

        sort_items(snapshot.opaque_items);
        for (const RenderItem* item : sorted_)
            draw_item(*item);

        if (!snapshot.transparent_items.empty())
        {
            set_blend_enabled(true);
            set_blend_function(BlendFunction::SRC_ALPHA,
                               BlendFunction::ONE_MINUS_SRC_ALPHA);
            sort_items(snapshot.transparent_items);
            for (const RenderItem* item : sorted_)
                draw_item(*item);
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

    void Renderer::sort_items(const std::vector<RenderItem>& items)
    {
        sorted_.clear();
        sorted_.reserve(items.size());
        for (const RenderItem& item : items)
            sorted_.push_back(&item);
        std::sort(sorted_.begin(), sorted_.end(),
                  [](const RenderItem* a, const RenderItem* b)
                  {
                      return a->sort_key() < b->sort_key();
                  });
    }

    void Renderer::draw_item(const RenderItem& item)
    {
        if (item.material() != current_material_)
            bind_material(item.material());

        const Mesh& mesh = resources_.get_mesh(item.mesh());
        state_.bind_vao(mesh.vao);

        // Orphan-and-upload the per-draw block; the binding point still
        // refers to this buffer.
        bind_buffer(BufferTarget::UNIFORM, per_draw_ubo_.id());
        set_buffer_data(BufferTarget::UNIFORM,
                        RENDER_ITEM_DATA_SIZE * sizeof(float),
                        item.data().data(), BufferUsage::DYNAMIC_DRAW);

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

        if (!material.parameter_data.empty())
        {
            bind_buffer(BufferTarget::UNIFORM, per_material_ubo_.id());
            set_buffer_data(
                BufferTarget::UNIFORM,
                static_cast<ptrdiff_t>(material.parameter_data.size()),
                material.parameter_data.data(),
                BufferUsage::DYNAMIC_DRAW);
        }
        else if (shader.has_material_block)
        {
            // Nothing to upload, but the shader reads the block — so it would
            // be drawn with whatever the previous material left in the UBO, or
            // with no data store at all if this is the first material of the
            // frame. Silently wrong colours are worse than a thrown error.
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
