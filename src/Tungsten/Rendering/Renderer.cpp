//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Rendering/Renderer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <vector>

#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlFramebuffer.hpp"
#include "Tungsten/Gl/GlRendering.hpp"
#include "Tungsten/Gl/GlStateManagement.hpp"
#include "GlPipelineBinder.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Resources/GlStateCache.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/RenderTarget.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/Resources/ResourceRefs.hpp"
#include "Tungsten/Resources/ShaderProgram.hpp"
#include "Tungsten/Resources/Texture.hpp"
#include "Tungsten/Resources/VertexAttribute.hpp"
#include "Tungsten/TungstenException.hpp"
#include "../Resources/UboBindings.hpp"

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

        void render(const RenderSnapshot& snapshot,
                    const RenderPassDescriptor& pass);

        /**
         * Binds the pass's target, sets the viewport and scissor, and applies
         * the attachments' load actions.
         */
        void begin_pass(const RenderPassDescriptor& pass);

        /** Applies the attachments' store actions. */
        void end_pass(const RenderPassDescriptor& pass);

        /**
         * Discards whichever attachments are marked DONT_CARE — on entry by
         * their load action, on exit by their store action.
         */
        void discard_attachments(const RenderPassDescriptor& pass,
                                 bool entering);

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
        /**
         * Fixed-function state, diffed per pipeline. Beside GlStateCache
         * rather than inside it — see GlPipelineBinder.
         */
        GlPipelineBinder pipelines;
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

    void Renderer::Members::render(const RenderSnapshot& snapshot,
                                   const RenderPassDescriptor& pass)
    {
        begin_pass(pass);
        bind_per_frame(snapshot);
        // Forget the previous frame's material: its slot may have been
        // destroyed and reused between frames.
        current_material = {};

        // Create a GL sampler on first use, this allows the constructor
        // to run before a context is current.
        default_sampler_id = resources.get_sampler_id({});

        // Forget the previous frame's pipeline too. The context is shared
        // with the older examples and with other event loops, so nothing
        // survives a frame boundary; starting empty also makes the first draw
        // of every frame state its whole pipeline, which is deterministic.
        pipelines.invalidate();

        // Both lists are sorted up front so every item's per-draw block can
        // go up in one upload; an item's index in sorted is its slot in the
        // per-draw buffer.
        sorted.clear();
        append_sorted(snapshot.opaque_items);
        append_sorted(snapshot.transparent_items);
        upload_per_draw_blocks();

        // One loop over both lists. The opaque run comes first because it was
        // appended first, and each item's depth, blend and raster state now
        // travels on its own pipeline rather than being toggled per pass.
        for (size_t i = 0; i < sorted.size(); ++i)
            draw_item(*sorted[i], i);

        end_pass(pass);
    }

    void Renderer::Members::begin_pass(const RenderPassDescriptor& pass)
    {
        const uint32_t framebuffer =
            pass.target ? resources.get_render_target(pass.target)
                              .framebuffer.id()
                        : 0;
        bind_framebuffer(FramebufferTarget::FRAMEBUFFER, framebuffer);

        set_viewport(pass.viewport);
        if (pass.scissor)
        {
            set_scissor_enabled(true);
            set_scissor(pass.scissor->x, pass.scissor->y,
                        pass.scissor->width, pass.scissor->height);
        }
        else
        {
            set_scissor_enabled(false);
        }

        // A clear writes through the depth mask, so it has to be open for the
        // depth clear to land. The first draw's pipeline sets it again.
        ClearBits bits = {};
        if (pass.color.load == LoadAction::CLEAR)
        {
            const auto& c = pass.color.clear_color;
            set_clear_color(c[0], c[1], c[2], c[3]);
            bits = bits | ClearBits::COLOR;
        }
        if (pass.depth && pass.depth->load == LoadAction::CLEAR)
        {
            set_clear_depth(pass.depth->clear_depth);
            set_depth_mask_enabled(true);
            pipelines.invalidate();
            bits = bits | ClearBits::DEPTH;
        }
        if (bits != ClearBits{})
            clear(bits);

        // DONT_CARE means the old contents are not needed. On a tiler that
        // saves loading the tile; on a desktop GL it is a no-op.
        discard_attachments(pass, true);
    }

    void Renderer::Members::end_pass(const RenderPassDescriptor& pass)
    {
        discard_attachments(pass, false);
    }

    void Renderer::Members::discard_attachments(
        const RenderPassDescriptor& pass, bool entering)
    {
        FrameBufferAttachment attachments[2];
        size_t count = 0;

        const auto discarded = [entering](LoadAction load, StoreAction store)
        {
            return entering ? load == LoadAction::DONT_CARE
                            : store == StoreAction::DONT_CARE;
        };

        if (discarded(pass.color.load, pass.color.store))
            attachments[count++] = FrameBufferAttachment::COLOR0;
        if (pass.depth && discarded(pass.depth->load, pass.depth->store))
            attachments[count++] = FrameBufferAttachment::DEPTH;

        if (count == 0)
            return;

        invalidate_framebuffer(FramebufferTarget::FRAMEBUFFER,
                               std::span(attachments, count),
                               !pass.target);
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
        // Pipeline first, then material: the pipeline owns the program, and
        // the material's textures and parameter buffer hang off it.
        const PipelineDescriptor& pipeline
            = resources.get_pipeline(item.pipeline());
        pipelines.bind(pipeline);

        if (item.material() != current_material)
            bind_material(item.material());

        const Mesh& mesh = resources.get_mesh(item.mesh());

        // A ref compare, because layouts are interned. Stricter than the old
        // semantic-mask test and cheaper: the mask only asked whether the
        // attributes the shader reads are present, so a mesh with the right
        // semantics packed differently was accepted and drew garbage. That a
        // layout supplies what the shader reads is checked once, at
        // register_pipeline.
        if (mesh.layout != pipeline.layout)
        {
            TUNGSTEN_THROW("Renderer: the mesh's vertex layout is not the one"
                " its pipeline was registered with.");
        }
        state.bind_vao(static_cast<uint32_t>(mesh.binding));

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
            draw_elements(pipeline.primitive, mesh.index_type,
                          static_cast<int32_t>(mesh.ebo.offset),
                          static_cast<int32_t>(mesh.ebo.count));
        }
        else if (!mesh.streams.empty())
        {
            // Non-indexed: draw the first stream's range; its offset is the
            // base vertex.
            draw_array(pipeline.primitive,
                       static_cast<int32_t>(mesh.streams[0].offset),
                       static_cast<int32_t>(mesh.streams[0].count));
        }
    }

    void Renderer::Members::bind_material(MaterialRef ref)
    {
        const Material& material = resources.get_material(ref);
        const ShaderProgram& shader = resources.get_shader(
            resources.get_pipeline(material.pipeline).shader);

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
            // A null ref leaves a slot deliberately empty — a material that
            // only fills a later slot still has to say something about the
            // earlier ones. Those get the same white texture as the trailing
            // slots below.
            if (!material.textures[i])
            {
                state.bind_texture(static_cast<int32_t>(unit),
                                   white_texture.id());
                state.bind_sampler(unit, default_sampler_id);
                continue;
            }

            const Texture& texture =
                resources.get_texture(material.textures[i]);
            state.bind_texture(static_cast<int32_t>(unit),
                               texture.gl_handle.id());
            // Every unit gets an explicit sampler. Leaving one unbound would
            // sample through whatever another subsystem last left on it — Neo
            // shares its context with Fonts and the legacy examples.
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

        current_material = ref;
    }

    Renderer::Renderer(ResourceManager& resources)
        : members_(std::make_unique<Members>(resources))
    {
    }

    Renderer::~Renderer() = default;

    void Renderer::render(const RenderSnapshot& snapshot,
                          const RenderPassDescriptor& pass)
    {
        members_->render(snapshot, pass);
    }
} // Tungsten
