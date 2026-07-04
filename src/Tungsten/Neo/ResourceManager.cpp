//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/ResourceManager.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace Tungsten
{
    ResourceManager::ResourceManager()
        : vao_cache_(
              [this](BufferArenaRef ref) -> const BufferArena&
              {
                  return arenas_.get(ref);
              },
              [this](VertexLayoutRef ref) -> const VertexLayout&
              {
                  return layout_registry_.get_layout(ref);
              }),
          shader_library_(
              [this](ShaderProgram program)
              {
                  return shaders_.insert(std::move(program));
              })
    {}

    BufferArenaRef ResourceManager::create_arena(BufferUsage usage,
                                                 uint16_t stride,
                                                 uint32_t capacity)
    {
        return arenas_.insert(BufferArena(usage, stride, capacity));
    }

    BufferArena& ResourceManager::get_arena(BufferArenaRef ref)
    {
        return arenas_.get(ref);
    }

    void ResourceManager::destroy_arena(BufferArenaRef ref)
    {
        vao_cache_.evict_for_arena(ref, deletions_);
        arenas_.erase(ref, [this](BufferArena&& arena)
        {
            deletions_.retire(arena.release_buffer());
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
        vao_cache_.rebuild_for_arena(ref);

        // The old buffer may still be referenced by in-flight draws
        // (or, with a render thread, by the snapshot being rendered),
        // so it is retired rather than deleted now.
        deletions_.retire(std::move(displaced));

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
        return layout_registry_.register_layout(layout);
    }

    const VertexLayout& ResourceManager::get_layout(VertexLayoutRef ref) const
    {
        return layout_registry_.get_layout(ref);
    }

    MeshRef ResourceManager::create_mesh(Mesh mesh)
    {
        return meshes_.insert(std::move(mesh));
    }

    Mesh& ResourceManager::get_mesh(MeshRef ref)
    {
        return meshes_.get(ref);
    }

    void ResourceManager::destroy_mesh(MeshRef ref)
    {
        meshes_.erase(ref, [this](Mesh&& mesh)
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
        return materials_.insert(std::move(material));
    }

    Material& ResourceManager::get_material(MaterialRef ref)
    {
        return materials_.get(ref);
    }

    void ResourceManager::destroy_material(MaterialRef ref)
    {
        // A material owns no GL object: its shader and textures are refs to
        // resources with their own lifetimes.
        materials_.erase(ref, [](Material&&) {});
    }

    TextureRef ResourceManager::create_texture(Texture texture)
    {
        return textures_.insert(std::move(texture));
    }

    Texture& ResourceManager::get_texture(TextureRef ref)
    {
        return textures_.get(ref);
    }

    void ResourceManager::destroy_texture(TextureRef ref)
    {
        textures_.erase(ref, [this](Texture&& texture)
        {
            deletions_.retire(std::move(texture.gl_handle));
        });
    }

    void ResourceManager::register_shader_family(ShaderFamilyId id,
                                                 ShaderFamily family)
    {
        shader_library_.register_family(id, std::move(family));
    }

    ShaderProgramRef ResourceManager::register_shader_variant(
        const ShaderVariantKey& key)
    {
        return shader_library_.register_variant(key);
    }

    ShaderProgram& ResourceManager::get_shader(ShaderProgramRef ref)
    {
        return shaders_.get(ref);
    }

    uint32_t ResourceManager::get_vao(std::span<const BufferArenaRef> vbo_arenas,
                                      BufferArenaRef ebo_arena,
                                      VertexLayoutRef layout)
    {
        return vao_cache_.get_vao(vbo_arenas, ebo_arena, layout);
    }

    void ResourceManager::begin_frame(uint64_t frame)
    {
        deletions_.begin_frame(frame);
    }

    void ResourceManager::collect_garbage(uint64_t completed_frame)
    {
        deletions_.collect_garbage(completed_frame);
    }
}
