//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/Renderer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <vector>

#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Neo/GlStateCache.hpp"
#include "Tungsten/Neo/Material.hpp"
#include "Tungsten/Neo/Mesh.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/ResourceRefs.hpp"
#include "Tungsten/Neo/ShaderProgram.hpp"
#include "Tungsten/Neo/Texture.hpp"
#include "Tungsten/Neo/VertexAttribute.hpp"
#include "Tungsten/TungstenException.hpp"
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

    struct Renderer::Members
    {
        explicit Members(ResourceManager& resources);

        void render(const RenderSnapshot& snapshot);

        void render_transparent_items(size_t start_index);

        /**
         * Uploads the per-frame block: camera matrices, camera position and
         * time, ambient light, and up to MAX_LIGHTS lights.
         */
        void bind_per_frame(const RenderSnapshot& snapshot);

        /**
         * Appends pointers to the items to sorted and orders that run by sort
         * key, leaving anything already there untouched.
         */
        void append_sorted(const std::vector<RenderItem>& items);

        /**
         * Packs every sorted item's per-draw block into one buffer, spaced by
         * per_draw_stride, and uploads it. An item's index in sorted is its
         * slot, so draw_item can bind its slice without a lookup.
         */
        void upload_per_draw_blocks();

        /**
         * Draws one item, binding its slice of the packed per-draw buffer.
         */
        void draw_item(const RenderItem& item, size_t slot);

        /**
         * Selects the item's program, uploads the material's parameter blob
         * to the per-material UBO, and binds its textures to consecutive
         * units. Called only when the material differs from the previous
         * item's, which the sort keeps rare.
         */
        void bind_material(MaterialRef ref);

        ResourceManager& resources;
        GlStateCache state;
        BufferHandle per_frame_ubo;
        BufferHandle per_draw_ubo;
        /**
         * 1×1 white, bound to every sampler unit the material leaves
         * unfilled: a sampler must see a complete texture even when the
         * shader's runtime flags never read it, and white is the
         * multiplicative identity if it is read anyway.
         */
        TextureHandle white_texture;
        /**
         * Sort scratch, reused across frames to avoid reallocation. Holds the
         * opaque run first, then the transparent one; an item's index here is
         * its slot in the per-draw buffer.
         */
        std::vector<const RenderItem*> sorted;
        /**
         * The packed per-draw blocks, staged here before the one upload.
         */
        std::vector<std::byte> staging;
        /**
         * Spacing between per-draw blocks: the block size rounded up to
         * GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, which every glBindBufferRange
         * offset has to be a multiple of.
         */
        size_t per_draw_stride = 0;
        MaterialRef current_material;
        /**
         * The required_attributes of current_material's shader, cached so the
         * per-draw check needs no lookup. Cleared with the material.
         */
        AttributeSemanticMask current_required_attributes = 0;
        /**
         * The sampler bound to units the current material leaves unfilled.
         * Re-resolved at the start of each frame.
         */
        uint32_t default_sampler_id = 0;
    };

    Renderer::Members::Members(ResourceManager& resources)
        : resources(resources)
    {
        per_frame_ubo = generate_buffer();
        per_draw_ubo = generate_buffer();

        // Only binding 0 is set once and left alone; its buffer never changes,
        // so there is no repeated bind for the state cache to elide. Binding 1
        // is re-pointed at each material's own parameter buffer, and binding 2
        // at each item's slice of the packed per-draw buffer.
        bind_buffer_base(BufferTarget::UNIFORM, PER_FRAME_UBO_BINDING,
                         per_frame_ubo.id());

        // Every bind_buffer_range offset must be a multiple of this, so the
        // per-draw blocks are spaced out to match. It is commonly 256 against
        // a 112-byte block; the padding is the price of one upload per frame.
        const auto alignment = size_t(get_uniform_buffer_offset_alignment());
        constexpr size_t block_size = RENDER_ITEM_DATA_SIZE * sizeof(float);
        per_draw_stride = (block_size + alignment - 1) / alignment * alignment;

        white_texture = generate_texture();
        bind_texture(TextureTarget::TEXTURE_2D, white_texture.id());
        constexpr uint8_t white[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D, 0, {1, 1},
                             RGBA_TEXTURE, white);
        // No texture parameters here: a sampler object is bound to every unit
        // this texture can land on, which overrides them. The default sampler's
        // mip_filter is what keeps this lone level-0 image complete.
    }

    void Renderer::Members::render(const RenderSnapshot& snapshot)
    {
        bind_per_frame(snapshot);
        // Forget the previous frame's material: its slot may have been
        // destroyed and reused between frames.
        current_material = {};
        current_required_attributes = 0;

        // Create a GL sampler on first use, this allows the constructor
        // to run before a context is current.
        default_sampler_id = resources.get_sampler_id({});

        // Both passes are sorted up front so every item's per-draw block can
        // go up in one upload; an item's index in sorted is its slot in the
        // per-draw buffer.
        sorted.clear();
        append_sorted(snapshot.opaque_items);
        const size_t opaque_count = sorted.size();
        append_sorted(snapshot.transparent_items);
        upload_per_draw_blocks();

        set_depth_test_enabled(true);
        // Stated rather than assumed, like the blend state beside it: the
        // context is shared with the legacy TextRenderer and the older
        // examples, and an opaque pass that silently stopped writing depth
        // would be very hard to place.
        set_depth_mask_enabled(true);
        set_blend_enabled(false);

        for (size_t i = 0; i < opaque_count; ++i)
            draw_item(*sorted[i], i);

        if (opaque_count != sorted.size())
            render_transparent_items(opaque_count);
    }

    void Renderer::Members::render_transparent_items(size_t start_index)
    {
        set_blend_enabled(true);
        set_blend_function(BlendFunction::SRC_ALPHA,
                           BlendFunction::ONE_MINUS_SRC_ALPHA);
        // The depth *test* stays on, so opaque geometry still occludes
        // these; only the write is masked. Blended surfaces are sorted
        // back-to-front precisely so they need not occlude each other.
        // Letting them write depth would mean the first one drawn punches
        // a hole in everything coplanar behind it.
        set_depth_mask_enabled(false);
        for (size_t i = start_index; i < sorted.size(); ++i)
            draw_item(*sorted[i], i);
        set_depth_mask_enabled(true);
        set_blend_enabled(false);
    }

    void Renderer::Members::bind_per_frame(const RenderSnapshot& snapshot)
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

        bind_buffer(BufferTarget::UNIFORM, per_frame_ubo.id());
        set_buffer_data(BufferTarget::UNIFORM, sizeof(block), &block,
                        BufferUsage::DYNAMIC_DRAW);
    }

    void Renderer::Members::append_sorted(const std::vector<RenderItem>& items)
    {
        const auto first = sorted.size();
        sorted.reserve(first + items.size());
        for (const RenderItem& item : items)
            sorted.push_back(&item);
        // Only the newly appended run is sorted: the two passes are ordered
        // independently and must stay in the order they were appended, opaque
        // before transparent.
        std::sort(sorted.begin() + ptrdiff_t(first), sorted.end(),
                  [](const RenderItem* a, const RenderItem* b)
                  {
                      return a->sort_key() < b->sort_key();
                  });
    }

    void Renderer::Members::upload_per_draw_blocks()
    {
        if (sorted.empty())
            return;

        // One block per item, each starting at a offset the driver will accept
        // for glBindBufferRange. The padding between them is wasted, but it is
        // bounded by the alignment (typically 256 bytes against a 112-byte
        // block) and buys one upload per frame instead of one per draw.
        staging.assign(sorted.size() * per_draw_stride, std::byte{});
        for (size_t i = 0; i < sorted.size(); ++i)
        {
            std::memcpy(staging.data() + i * per_draw_stride,
                        sorted[i]->data().data(),
                        RENDER_ITEM_DATA_SIZE * sizeof(float));
        }

        // Respecify rather than sub-update: handing the driver a fresh store
        // each frame lets it hand back memory the GPU is not still reading,
        // which is the cheap way to avoid a stall without fences.
        bind_buffer(BufferTarget::UNIFORM, per_draw_ubo.id());
        set_buffer_data(BufferTarget::UNIFORM,
                        static_cast<ptrdiff_t>(staging.size()),
                        staging.data(), BufferUsage::DYNAMIC_DRAW);
    }

    void Renderer::Members::draw_item(const RenderItem& item, size_t slot)
    {
        if (item.material() != current_material)
            bind_material(item.material());

        const Mesh& mesh = resources.get_mesh(item.mesh());

        if ((mesh.semantics & current_required_attributes)
            != current_required_attributes)
        {
            TUNGSTEN_THROW("Renderer: the mesh does not provide every vertex"
                " attribute its material's shader reads.");
        }
        state.bind_vao(mesh.vao);

        // The block was uploaded with all the others; point binding 2 at this
        // item's slice of that buffer rather than rewriting it.
        bind_buffer_range(BufferTarget::UNIFORM, PER_DRAW_UBO_BINDING,
                          per_draw_ubo.id(),
                          static_cast<ptrdiff_t>(slot * per_draw_stride),
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

    void Renderer::Members::bind_material(MaterialRef ref)
    {
        const Material& material = resources.get_material(ref);
        const ShaderProgram& shader = resources.get_shader(material.shader);

        state.use_program(shader.gl_handle.id());

        if (material.ubo)
        {
            // The parameters were uploaded when the material was created, so
            // switching materials only moves the binding point onto another
            // buffer. Consecutive items sharing a material bind nothing.
            state.bind_material_ubo(material.ubo.id());
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
            const auto unit = static_cast<uint32_t>(i);
            const Texture& texture =
                resources.get_texture(material.textures[i]);
            state.bind_texture(static_cast<int32_t>(unit),
                               texture.gl_handle.id());
            // Every unit gets an explicit sampler. Leaving one unbound would
            // sample through whatever another subsystem last left on it — Neo
            // shares its context with TextRenderer and the legacy examples.
            state.bind_sampler(unit,
                               resources.get_sampler_id(texture.sampler));
        }

        // The program samples units [0, sampler_count) whether or not the
        // material fills them; the unfilled ones get the white texture.
        for (auto i = static_cast<uint32_t>(material.textures.size());
             i < shader.sampler_count; ++i)
        {
            state.bind_texture(static_cast<int32_t>(i), white_texture.id());
            state.bind_sampler(i, default_sampler_id);
        }

        current_required_attributes = shader.required_attributes;
        current_material = ref;
    }

    Renderer::Renderer(ResourceManager& resources)
        : members_(std::make_unique<Members>(resources))
    {
    }

    Renderer::~Renderer() = default;

    void Renderer::render(const RenderSnapshot& snapshot)
    {
        members_->render(snapshot);
    }
} // Tungsten
