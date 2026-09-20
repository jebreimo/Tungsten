//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/ResourceManager.hpp"

#include <algorithm>
#include <ranges>
#include <limits>
#include <utility>

#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/ResourceRefs.hpp"
#include "Tungsten/Resources/SharedBuffer.hpp"
#include "Tungsten/Gl/GlStateEpoch.hpp"
#include "Tungsten/Resources/RenderTarget.hpp"
#include "Tungsten/Resources/Texture.hpp"
#include "BufferArena.hpp"
#include "DeletionQueue.hpp"
#include "GenerationalPool.hpp"
#include "LayoutRegistry.hpp"
#include "PipelineRegistry.hpp"
#include "SamplerRegistry.hpp"
#include "ShaderLibrary.hpp"
#include "VaoCache.hpp"

namespace Tungsten
{
    struct ResourceManager::Members
    {
        GenerationalPool<BufferArena> arenas;
        GenerationalPool<Mesh> meshes;
        GenerationalPool<Material> materials;
        GenerationalPool<ShaderProgram> shaders;
        GenerationalPool<Texture> textures;
        GenerationalPool<RenderTarget> render_targets;
        LayoutRegistry layout_registry;
        PipelineRegistry pipeline_registry;
        SamplerRegistry sampler_registry;
        DeletionQueue deletions;
        VaoCache vao_cache;
        ShaderLibrary shader_library;

        Members()
            : vao_cache(
                  [this](BufferArenaRef ref) -> const BufferArena&
                  {
                      return arenas.get(ref);
                  },
                  [this](VertexLayoutRef ref) -> const VertexLayout&
                  {
                      return layout_registry.get_layout(ref);
                  }),
              shader_library(
                  [this](ShaderProgram program)
                  {
                      return shaders.insert(std::move(program));
                  })
        {
        }
    };

    ResourceManager::ResourceManager()
        : members_(std::make_unique<Members>())
    {
    }

    ResourceManager::~ResourceManager() = default;

    ResourceManager::ResourceManager(ResourceManager&& rhs) noexcept
        : members_(std::move(rhs.members_))
    {
    }

    ResourceManager& ResourceManager::operator=(ResourceManager&& rhs) noexcept
    {
        members_ = std::move(rhs.members_);
        return *this;
    }

    BufferArenaRef ResourceManager::create_arena(BufferUsage usage,
                                                 uint16_t stride,
                                                 uint32_t capacity)
    {
        return members_->arenas.insert(BufferArena(usage, stride, capacity));
    }

    BufferArena& ResourceManager::get_arena(BufferArenaRef ref)
    {
        return members_->arenas.get(ref);
    }

    void ResourceManager::destroy_arena(BufferArenaRef ref)
    {
        members_->vao_cache.evict_for_arena(ref, members_->deletions);
        members_->arenas.erase(ref, [this](BufferArena&& arena)
        {
            members_->deletions.retire(arena.release_buffer());
        });
    }

    SharedBuffer ResourceManager::allocate(BufferArenaRef ref, uint32_t count)
    {
        BufferArena& arena = get_arena(ref);
        if (const auto offset = arena.allocate(count))
            return SharedBuffer{ref, *offset, count};

        // Full: grow the arena, then retry. Doubling (or sizing to the request
        // with headroom when the arena is empty) keeps the amortized cost of
        // repeated allocations constant; grow() bit_ceils the value anyway.
        const auto new_capacity = std::max(
            static_cast<uint64_t>(arena.capacity()) * 2,
            static_cast<uint64_t>(count) * (arena.empty() ? 1 : 2));
        if (new_capacity > std::numeric_limits<uint32_t>::max())
            TUNGSTEN_THROW("ResourceManager: arena capacity overflow.");

        BufferHandle displaced = arena.grow(static_cast<uint32_t>(new_capacity));

        // The arena's buffer id changed. Re-point every VAO that baked in
        // this arena's buffer before the next draw.
        members_->vao_cache.rebuild_for_arena(ref);

        // The old buffer may still be referenced by in-flight draws
        // (or, with a render thread, by the snapshot being rendered),
        // so it is retired rather than deleted now.
        members_->deletions.retire(std::move(displaced));

        const auto offset = arena.allocate(count);
        if (!offset)
            TUNGSTEN_THROW("ResourceManager: allocation failed after grow.");
        return SharedBuffer{ref, *offset, count};
    }

    void ResourceManager::free(const SharedBuffer& slice)
    {
        get_arena(slice.arena).free(slice.offset);
    }

    void ResourceManager::upload(const SharedBuffer& slice, const void* data,
                                 size_t size)
    {
        BufferArena& arena = get_arena(slice.arena);
        if (size != size_t(slice.count) * arena.stride())
            TUNGSTEN_THROW("ResourceManager: data size does not match slice.");

        bind_buffer(BufferTarget::COPY_WRITE, arena.buffer_id());
        set_buffer_subdata(BufferTarget::COPY_WRITE,
                           ptrdiff_t(slice.offset) * arena.stride(),
                           static_cast<ptrdiff_t>(size), data);
    }

    VertexLayoutRef ResourceManager::register_layout(const VertexLayout& layout)
    {
        return members_->layout_registry.register_layout(layout);
    }

    const VertexLayout& ResourceManager::get_layout(VertexLayoutRef ref) const
    {
        return members_->layout_registry.get_layout(ref);
    }

    SamplerRef ResourceManager::register_sampler(const SamplerDescriptor& descriptor)
    {
        return members_->sampler_registry.register_sampler(descriptor);
    }

    uint32_t ResourceManager::get_sampler_id(SamplerRef ref)
    {
        return members_->sampler_registry.get_sampler_id(ref);
    }

    const SamplerDescriptor&
    ResourceManager::get_sampler_descriptor(SamplerRef ref) const
    {
        return members_->sampler_registry.get_descriptor(ref);
    }

    SamplerRef ResourceManager::default_sampler()
    {
        return members_->sampler_registry.default_sampler();
    }

    MeshRef ResourceManager::create_mesh(Mesh mesh)
    {
        if (mesh.layout)
        {
            const VertexLayout& layout = get_layout(mesh.layout);
            validate_mesh_layout(mesh, layout);
            mesh.semantics = layout.semantics();
        }
        // The binding is derived here rather than supplied by the caller, so
        // a mesh cannot be created naming arenas its binding does not cover.
        mesh.binding = get_geometry_binding(mesh);
        return members_->meshes.insert(std::move(mesh));
    }

    GeometryBinding ResourceManager::get_geometry_binding(const Mesh& mesh)
    {
        if (!mesh.layout)
            return GeometryBinding::NONE;

        std::vector<BufferArenaRef> vbo_arenas;
        vbo_arenas.reserve(mesh.streams.size());
        for (const SharedBuffer& stream : mesh.streams)
            vbo_arenas.push_back(stream.arena);

        // A mesh whose slices are filled in after creation has nothing to
        // bind yet; validate_mesh_layout permits that, so this must too.
        const bool has_arena = std::ranges::any_of(
            vbo_arenas, [](BufferArenaRef ref) { return bool(ref); });
        if (!has_arena && !mesh.ebo.arena)
            return GeometryBinding::NONE;

        return GeometryBinding(members_->vao_cache.get_vao(
            vbo_arenas, mesh.ebo.arena, mesh.layout));
    }

    void ResourceManager::validate_mesh_layout(const Mesh& mesh,
                                               const VertexLayout& layout)
    {
        for (const VertexAttribute& attribute : layout.attributes)
        {
            if (attribute.stream_index >= mesh.streams.size())
            {
                TUNGSTEN_THROW(
                    "ResourceManager: the mesh's layout reads a vertex stream"
                    " the mesh does not have.");
            }

            const SharedBuffer& stream = mesh.streams[attribute.stream_index];
            // A caller may create the mesh and fill its slices afterwards.
            if (!stream.arena)
                continue;

            // Make sure the attribute's end offset isn't greater than the
            // stream buffer's stride.
            const auto stride = size_t(get_arena(stream.arena).stride());
            const auto end = size_t(attribute.offset_in_stream)
                             + byte_size(attribute);
            if (end > stride)
            {
                TUNGSTEN_THROW(
                    "ResourceManager: a vertex attribute extends past the end"
                    " of its stream's vertex.");
            }
        }
    }

    Mesh& ResourceManager::get_mesh(MeshRef ref)
    {
        return members_->meshes.get(ref);
    }

    void ResourceManager::destroy_mesh(MeshRef ref)
    {
        members_->meshes.erase(ref, [this](Mesh&& mesh)
        {
            for (const auto& stream : mesh.streams)
                free(stream);
            // The ebo is null for meshes drawn with array draws.
            if (mesh.ebo.arena)
                free(mesh.ebo);
        });
    }

    MaterialRef ResourceManager::create_material(Material material)
    {
        validate_material_textures(material);
        upload_material_parameters(material);
        // Folded once here so SnapshotBuilder never resolves the pipeline in
        // its per-renderable loop, and so a material may still be created
        // before its pipeline exists.
        if (material.pipeline)
            material.queue = get_pipeline(material.pipeline).queue;
        return members_->materials.insert(std::move(material));
    }

    void ResourceManager::validate_material_textures(const Material& material)
    {
        // A material may be built before its pipeline is registered, in which
        // case there is no slot list to check against yet.
        if (!material.pipeline)
            return;

        const ShaderProgram& shader
            = get_shader(get_pipeline(material.pipeline).shader);
        const ShaderFamily& family =
            members_->shader_library.get_family(shader.variant_key.family);

        const auto count = std::min(material.textures.size(),
                                    family.samplers.size());
        for (size_t i = 0; i < count; ++i)
        {
            // Null refs are the renderer's white fallback, which reads the
            // same in either encoding.
            if (!material.textures[i])
                continue;

            const Texture& texture = get_texture(material.textures[i]);
            if (texture.content == family.samplers[i].content)
                continue;

            TUNGSTEN_THROW(
                "Texture " + std::to_string(i) + " of this material is "
                + (texture.content == TextureContent::COLOR ? "colour" : "data")
                + ", but sampler '" + family.samplers[i].name + "' expects "
                + (family.samplers[i].content == TextureContent::COLOR
                       ? "colour" : "data")
                + ". Colour textures are sRGB-decoded when sampled and data"
                  " textures are not, so the two are not interchangeable.");
        }
    }

    void ResourceManager::update_material_parameters(
        MaterialRef ref, std::span<const std::byte> parameters)
    {
        Material& material = members_->materials.get(ref);
        material.parameter_data.assign(parameters.begin(), parameters.end());
        upload_material_parameters(material);
    }

    void ResourceManager::upload_material_parameters(Material& material)
    {
        if (material.parameter_data.empty())
            return;

        if (!material.ubo)
            material.ubo = generate_buffer();

        // Respecify rather than sub-update: the blob's size can change when a
        // material is pointed at a different shader, and this runs on edits,
        // not per frame.
        bind_buffer(BufferTarget::UNIFORM, material.ubo.id());
        set_buffer_data(
            BufferTarget::UNIFORM,
            static_cast<ptrdiff_t>(material.parameter_data.size()),
            material.parameter_data.data(),
            BufferUsage::STATIC_DRAW);
    }

    Material& ResourceManager::get_material(MaterialRef ref)
    {
        return members_->materials.get(ref);
    }

    void ResourceManager::destroy_material(MaterialRef ref)
    {
        // The parameter UBO is the material's own; its shader and textures are
        // refs to resources with their own lifetimes.
        members_->materials.erase(ref, [this](Material&& material)
        {
            members_->deletions.retire(std::move(material.ubo));
        });
    }

    TextureRef ResourceManager::create_texture(const TextureImage2D& image)
    {
        Texture texture;
        texture.gl_handle = generate_texture();
        texture.width = uint32_t(image.size.x());
        texture.height = uint32_t(image.size.y());
        texture.format = image.format.format;
        texture.content = image.content;
        texture.sampler = image.sampler;

        auto format = image.format;
        format.color_space = to_color_space(image.content, format);

        bind_texture(TextureTarget::TEXTURE_2D, texture.gl_handle.id());
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D, 0, image.size,
                             format, image.pixels);

        return members_->textures.insert(std::move(texture));
    }

    TextureRef ResourceManager::create_texture(Texture texture)
    {
        return members_->textures.insert(std::move(texture));
    }

    Texture& ResourceManager::get_texture(TextureRef ref)
    {
        return members_->textures.get(ref);
    }

    void ResourceManager::destroy_texture(TextureRef ref)
    {
        members_->textures.erase(ref, [this](Texture&& texture)
        {
            members_->deletions.retire(std::move(texture.gl_handle));
        });
    }

    RenderTargetRef ResourceManager::create_render_target(TextureRef color,
                                                          bool with_depth)
    {
        const Texture& color_texture = get_texture(color);

        RenderTarget target;
        target.color = color;
        target.size = {int32_t(color_texture.width),
                       int32_t(color_texture.height)};

        if (with_depth)
        {
            // A depth *texture*, not a renderbuffer: it is what Metal and
            // Vulkan attach, and it can be sampled afterwards. No pixels —
            // the target's own passes are what fill it.
            target.depth = create_texture(TextureImage2D{
                .size = target.size,
                .format = {TextureFormat::DEPTH, TextureValueType::UINT32},
                .content = TextureContent::DATA,
                .pixels = nullptr
            });
        }

        target.framebuffer = generate_framebuffer();
        bind_framebuffer(FramebufferTarget::FRAMEBUFFER,
                         target.framebuffer.id());
        framebuffer_texture_2d(FramebufferTarget::FRAMEBUFFER,
                               FrameBufferAttachment::COLOR0,
                               TextureTarget2D::TEXTURE_2D,
                               color_texture.gl_handle.id());
        if (target.depth)
        {
            framebuffer_texture_2d(FramebufferTarget::FRAMEBUFFER,
                                   FrameBufferAttachment::DEPTH,
                                   TextureTarget2D::TEXTURE_2D,
                                   get_texture(target.depth).gl_handle.id());
        }
        assert_framebuffer_complete(FramebufferTarget::FRAMEBUFFER);
        bind_framebuffer(FramebufferTarget::FRAMEBUFFER, 0);
        // Building the target bound a framebuffer and a texture behind the
        // caches' backs, exactly as VAO baking does.
        notify_gl_state_changed();

        return members_->render_targets.insert(std::move(target));
    }

    RenderTarget& ResourceManager::get_render_target(RenderTargetRef ref)
    {
        return members_->render_targets.get(ref);
    }

    void ResourceManager::destroy_render_target(RenderTargetRef ref)
    {
        members_->render_targets.erase(ref, [this](RenderTarget&& target)
        {
            members_->deletions.retire(std::move(target.framebuffer));
            // The depth texture was created here, so it goes here. The colour
            // texture belongs to the caller.
            if (target.depth)
                destroy_texture(target.depth);
        });
    }

    void ResourceManager::register_shader_family(ShaderFamilyId id,
                                                 ShaderFamily family)
    {
        members_->shader_library.register_family(id, std::move(family));
    }

    ShaderProgramRef ResourceManager::register_shader_variant(
        const ShaderVariantKey& key)
    {
        return members_->shader_library.register_variant(key);
    }

    ShaderProgram& ResourceManager::get_shader(ShaderProgramRef ref)
    {
        return members_->shaders.get(ref);
    }

    PipelineRef ResourceManager::register_pipeline(
        const PipelineDescriptor& descriptor)
    {
        // Checked here rather than per draw: the pipeline is the one place
        // that names a shader and a vertex layout together, so this is where
        // a mismatch between them can be reported against both.
        const auto required = get_shader(descriptor.shader).required_attributes;
        const auto provided = get_layout(descriptor.layout).semantics();
        if ((provided & required) != required)
        {
            TUNGSTEN_THROW("ResourceManager: the pipeline's vertex layout does"
                " not provide every vertex attribute its shader reads.");
        }
        return members_->pipeline_registry.register_pipeline(descriptor);
    }

    const PipelineDescriptor&
    ResourceManager::get_pipeline(PipelineRef ref) const
    {
        return members_->pipeline_registry.get_pipeline(ref);
    }

    void ResourceManager::begin_frame(uint64_t frame)
    {
        members_->deletions.begin_frame(frame);
    }

    void ResourceManager::collect_garbage(uint64_t completed_frame)
    {
        members_->deletions.collect_garbage(completed_frame);
    }
}
