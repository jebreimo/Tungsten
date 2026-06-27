//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/ResourceManager.hpp"

namespace Tungsten
{
    BufferArenaRef ResourceManager::create_arena(BufferUsage usage, uint16_t stride,
        uint32_t capacity)
    {
        const uint32_t index = acquire_arena_slot();
        ArenaSlot& slot = arenas_[index];
        slot.arena.emplace(usage, stride, capacity);
        return BufferArenaRef{index, slot.generation};
    }

    BufferArena& ResourceManager::get_arena(BufferArenaRef ref)
    {
        if (ref.index >= arenas_.size())
            TUNGSTEN_THROW("ResourceManager: arena ref index out of range.");
        ArenaSlot& slot = arenas_[ref.index];
        if (!slot.arena || slot.generation != ref.generation)
            TUNGSTEN_THROW("ResourceManager: stale arena ref.");
        return *slot.arena;
    }

    SharedBuffer ResourceManager::allocate(BufferArenaRef ref, uint32_t count)
    {
        BufferArena::Allocation alloc = get_arena(ref).allocate(count);
        // The arena returns a byte offset; pair it with the ref (which only
        // this owner knows) and the count to form the SharedBuffer.
        SharedBuffer slice{ref, alloc.offset, count};

        if (alloc.retired_buffer)
        {
            // The arena's buffer id changed. Re-point every VAO that baked in
            // this arena's buffer before the next draw.
            rebuild_vaos_for_arena(ref);

            // The old buffer may still be referenced by in-flight draws
            // (or, with a render thread, by the snapshot being rendered),
            // so it is retired rather than deleted now.
            retire(std::move(alloc.retired_buffer));
        }
        return slice;
    }

    void ResourceManager::free(const SharedBuffer& slice)
    {
        get_arena(slice.arena).free(slice.offset);
    }

    uint32_t ResourceManager::get_vao(std::span<const BufferArenaRef> vbo_arenas,
                                      BufferArenaRef ebo_arena,
                                      VertexLayoutRef layout)
    {
        VaoKey key{{vbo_arenas.begin(), vbo_arenas.end()}, ebo_arena, layout};
        for (const auto& entry : vaos_)
        {
            if (entry.key == key)
                return entry.vao.id();
        }
        return build_vao(std::move(key));
    }

    uint32_t ResourceManager::build_vao(VaoKey key)
    {
        VertexArrayHandle vao = generate_vertex_array();
        const uint32_t id = vao.id();

        bind_vertex_array(id);
        // TODO: bake the bindings. For each vertex stream binding, bind the VBO
        // arena's current buffer (get_arena(ref).buffer_id()) at offset 0 with
        // the layout's per-binding stride, set the attribute formats from the
        // resolved VertexLayout, then bind the element buffer
        // (get_arena(key.ebo_arena).buffer_id()). Base offsets are not baked
        // (see get_vao / §3, §7). Needs the layout registry, not sketched here.
        bind_vertex_array(0);

        vaos_.push_back({std::move(key), std::move(vao)});
        return id;
    }

    void ResourceManager::rebuild_vaos_for_arena(BufferArenaRef ref)
    {
        const uint32_t new_buffer = get_arena(ref).buffer_id();
        for (auto& entry : vaos_)
        {
            if (!key_references(entry.key, ref))
                continue;

            bind_vertex_array(entry.vao.id());
            // TODO: re-point this VAO's binding(s) that read `ref`'s arena (and
            // its element binding, if ref == ebo_arena) at `new_buffer`. The VAO
            // id is unchanged, so meshes holding it stay valid — only the baked
            // buffer object is swapped.
            (void)new_buffer;
        }
        bind_vertex_array(0);
    }

    bool ResourceManager::key_references(const VaoKey& key, BufferArenaRef ref)
    {
        if (key.ebo_arena == ref)
            return true;
        for (const auto& arena : key.vbo_arenas)
        {
            if (arena == ref)
                return true;
        }
        return false;
    }

    void ResourceManager::begin_frame(uint64_t frame)
    {
        current_frame_ = frame;
    }

    void ResourceManager::collect_garbage(uint64_t completed_frame)
    {
        std::erase_if(retired_buffers_, [&](const RetiredBuffer& r) {
            return r.frame <= completed_frame;
        });
    }

    uint32_t ResourceManager::acquire_arena_slot()
    {
        if (!free_arena_slots_.empty())
        {
            const uint32_t index = free_arena_slots_.back();
            free_arena_slots_.pop_back();
            return index;
        }
        arenas_.emplace_back();
        return static_cast<uint32_t>(arenas_.size() - 1);
    }

    void ResourceManager::retire(BufferHandle buffer)
    {
        retired_buffers_.push_back({std::move(buffer), current_frame_});
    }
}
