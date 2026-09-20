//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include "Tungsten/Gpu/GpuTypes.hpp"
#include "ResourceRefs.hpp"
#include "PipelineDescriptor.hpp"
#include "ShaderFamily.hpp"
#include "SharedBuffer.hpp"

namespace Tungsten
{
    // Defined in Mesh.hpp, which this header deliberately does not include.
    enum class GeometryBinding : uint32_t;

    /**
     * The single owner of GPU resources, and the only place that knows a
     * resource's logical ref ({index, generation}).
     *
     * It is a thin facade: every public method forwards to one of five
     * GenerationalPools (Mesh, Material, ShaderProgram, Texture, BufferArena)
     * or to a focused collaborator it owns — LayoutRegistry (layout interning),
     * VaoCache (shared VAOs), ShaderLibrary (variant compilation),
     * DeletionQueue (frame-tagged deferred deletion). The facade
     * wires them together: VaoCache resolves arenas and layouts through the
     * arena pool and the registry, ShaderLibrary inserts compiled programs
     * into the shader pool, and everything that takes a GL object out of
     * service retires it into the one DeletionQueue.
     */
    class ResourceManager
    {
    public:
        ResourceManager();

        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        ResourceManager(ResourceManager&&) noexcept;
        ResourceManager& operator=(ResourceManager&&) noexcept;

        BufferArenaRef create_arena(BufferUsage usage, uint16_t stride,
                                    uint32_t capacity);

        [[nodiscard]]
        BufferArena& get_arena(BufferArenaRef ref);

        /**
         * Destroys an arena: every cached VAO that binds it is evicted and
         * its GL buffer is retired. Only valid once no live mesh draws from
         * the arena.
         */
        void destroy_arena(BufferArenaRef ref);

        /**
         * Allocates a slice from an arena and stamps its identity: this is
         * the one place the BufferArenaRef is known, so this is where the
         * arena's bare unit offset is paired with the ref and the count to
         * form the SharedBuffer. If the arena is full it is grown here: the
         * displaced GL buffer is retired, the affected VAOs are re-pointed in
         * place, and the allocation is retried.
         */
        SharedBuffer allocate(BufferArenaRef ref, uint32_t count);

        /**
         * Returns a slice's range to its arena.
         */
        void free(const SharedBuffer& slice);

        /**
         * Uploads `size` bytes into the slice's range of its arena's GL
         * buffer. The data must fill the slice exactly: size must equal
         * slice.count times the arena's stride. Uses the COPY_WRITE target,
         * so no VAO's element binding is disturbed.
         */
        void upload(const SharedBuffer& slice, const void* data, size_t size);

        VertexLayoutRef register_layout(const VertexLayout& layout);

        [[nodiscard]]
        const VertexLayout& get_layout(VertexLayoutRef ref) const;

        /**
         * Interns a sampler descriptor, creating its GL sampler object the
         * first time that descriptor is seen. Put the returned ref on a Texture
         * to say how it is sampled.
         */
        SamplerRef register_sampler(const SamplerDescriptor& descriptor);

        /**
         * The GL id the renderer binds for a sampler ref. A null ref resolves
         * to the default sampler.
         */
        [[nodiscard]]
        uint32_t get_sampler_id(SamplerRef ref);

        /**
         * Resolves a sampler ref to the descriptor it was interned from.
         */
        [[nodiscard]]
        const SamplerDescriptor& get_sampler_descriptor(SamplerRef ref) const;

        /**
         * The sampler a texture with a null ref is drawn with. Created on first
         * use, so this must not be called before a GL context exists.
         */
        SamplerRef default_sampler();

        MeshRef create_mesh(Mesh mesh);

        [[nodiscard]]
        Mesh& get_mesh(MeshRef ref);

        /**
         * Destroys a mesh and returns its slices (streams and ebo) to their
         * arenas. A mesh owns no GL object — its VAO belongs to the VaoCache.
         */
        void destroy_mesh(MeshRef ref);

        /**
         * Takes ownership of a material and uploads its parameter_data into a
         * UBO of its own, so drawing it later only binds that buffer.
         */
        MaterialRef create_material(Material material);

        [[nodiscard]]
        Material& get_material(MaterialRef ref);

        /**
         * Replaces the material's parameters and re-uploads them. Mutating
         * Material::parameter_data directly has no effect on what is drawn —
         * the buffer is the source of truth once the material exists.
         */
        void update_material_parameters(MaterialRef ref,
                                        std::span<const std::byte> parameters);

        void destroy_material(MaterialRef ref);

        /**
         * Uploads an image and creates the texture that owns it. The image's
         * `content` picks the internal format — sRGB for 8-bit colour, linear
         * for everything else — and is recorded on the Texture, so the two
         * cannot disagree. This is the way to create a texture from pixels.
         */
        TextureRef create_texture(const TextureImage2D& image);

        /**
         * Adopts a texture whose GL object the caller allocated and filled
         * itself, for the cases the overload above cannot express — an atlas
         * built up incrementally, or a render target. The caller is then also
         * responsible for `content` matching the internal format it chose.
         */
        TextureRef create_texture(Texture texture);

        [[nodiscard]]
        Texture& get_texture(TextureRef ref);

        /**
         * Destroys a texture. Its ref is revoked immediately; the GL texture
         * is deleted once no in-flight frame can reference it.
         */
        void destroy_texture(TextureRef ref);

        /**
         * Creates a render target that draws into @a color, adding a depth
         * texture of the same size when @a with_depth.
         *
         * The colour texture is not owned by the target; it stays in the
         * texture pool so later passes can sample it. The depth texture is
         * created here and named by the target, and is destroyed with it.
         */
        RenderTargetRef create_render_target(TextureRef color,
                                             bool with_depth = false);

        [[nodiscard]]
        RenderTarget& get_render_target(RenderTargetRef ref);

        /**
         * Retires the target's framebuffer and its depth texture through the
         * deletion queue. The colour texture is left alone: the caller created
         * it and may still be sampling it.
         */
        void destroy_render_target(RenderTargetRef ref);

        void register_shader_family(ShaderFamilyId id, ShaderFamily family);

        /**
         * Returns the program for the key, compiling and inserting it into
         * the shader pool on first use.
         */
        ShaderProgramRef register_shader_variant(const ShaderVariantKey& key);

        [[nodiscard]]
        ShaderProgram& get_shader(ShaderProgramRef ref);

        /**
         * Returns the ref for a pipeline equal to @a descriptor, interning it
         * on first use.
         *
         * Validates once, here, that the descriptor's vertex layout provides
         * every attribute semantic its shader reads — so a mismatch is caught
         * where the two are named together, not on each draw.
         */
        PipelineRef register_pipeline(const PipelineDescriptor& descriptor);

        [[nodiscard]]
        const PipelineDescriptor& get_pipeline(PipelineRef ref) const;

        /**
         * Calls once per frame with the id of the frame about to be built /
         * submitted; retired GL objects are tagged with it.
         */
        void begin_frame(uint64_t frame);

        /**
         * Deletes every GL object retired in a frame the GPU has finished
         * with. Single-threaded, completed_frame is the just-drawn frame;
         * with a render thread it is the latest passed fence.
         */
        void collect_garbage(uint64_t completed_frame);

    private:
        /**
         * Throws unless every attribute of the mesh's layout names a stream the
         * mesh has and ends within that stream's arena stride. Called by
         * create_mesh.
         */
        void validate_mesh_layout(const Mesh& mesh, const VertexLayout& layout);

        /**
         * Returns the geometry binding for the mesh's stream and element
         * arenas under its layout, creating and caching it on first use.
         * NONE when the mesh has no layout or none of its streams name an
         * arena yet, which is legal: a caller may fill the slices afterwards.
         */
        [[nodiscard]]
        GeometryBinding get_geometry_binding(const Mesh& mesh);

        /**
         * Throws if a texture's content disagrees with the sampler slot its
         * position binds it to, which would mean sampling colour as data or
         * the other way round. Called by create_material.
         */
        void validate_material_textures(const Material& material);

        /**
         * Creates the material's UBO if it has none yet and uploads its
         * parameter_data into it. Does nothing for a material with no
         * parameters, which is left without a buffer.
         */
        static void upload_material_parameters(Material& material);

        struct Members;
        std::unique_ptr<Members> members_;
    };
}
