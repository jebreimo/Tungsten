//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <vector>
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlVertexArray.hpp"
#include "Tungsten/TungstenException.hpp"
#include "BufferArena.hpp"
#include "ResourceRefs.hpp"
#include "SharedBuffer.hpp"

namespace Tungsten
{
    // SKETCH: only the buffer-arena surface of ResourceManager is shown here.
    // The mesh / material / shader / texture slot tables and the VAO cache
    // follow the same generational-slot pattern and are omitted.
    //
    // ResourceManager is the single owner of GPU resources and the only place
    // that knows a resource's logical ref ({index, generation}). It therefore:
    //   - stamps the BufferArenaRef onto the SharedBuffers an arena hands out,
    //   - drives buffer growth and retires the displaced GL buffers, and
    //   - (would) rebuild the VAOs invalidated when an arena's buffer id moves.
    class ResourceManager
    {
    public:
        // ---- Buffer arenas ------------------------------------------------

        BufferArenaRef create_arena(BufferUsage usage, uint16_t stride,
                                    uint32_t capacity);

        // Resolves a ref to its arena, validating the generation so a stale ref
        // (held across a destroy + slot reuse) throws instead of aliasing.
        BufferArena& get_arena(BufferArenaRef ref);

        // Allocates a slice from an arena and stamps its identity. This is the
        // one place the BufferArenaRef is known, so this is where it is written
        // onto the SharedBuffer. If the allocation grew the arena, the old GL
        // buffer is retired here and the affected VAOs are rebuilt.
        SharedBuffer allocate(BufferArenaRef ref, uint32_t count);

        void free(const SharedBuffer& slice);

        // ---- VAO cache ----------------------------------------------------

        // Returns a VAO that binds the given vertex-buffer arenas and element
        // arena with the given layout, creating and caching it on first use.
        //
        // A VAO bakes in its buffer-object bindings (each vertex stream's VBO
        // and the element buffer) and the attribute format, so it is valid for
        // exactly one (vbo arenas, ebo arena, layout) combination and is shared
        // by every mesh with that combination (§7). Per-mesh base offsets are
        // NOT baked: array draws pass them as `first`, indexed draws use
        // absolute (rebased) indices, so meshes differing only by offset reuse
        // one VAO.
        //
        // The cache OWNS the VAO (returning a non-owning id, not a
        // VertexArrayHandle): since VAOs are shared, no single Mesh can own one.
        // A Mesh therefore stores the id returned here, not a VertexArrayHandle.
        // The key is the arena *refs* (stable identity), not the live buffer
        // ids, so the entry survives a grow — see rebuild_vaos_for_arena.
        uint32_t get_vao(std::span<const BufferArenaRef> vbo_arenas,
                         BufferArenaRef ebo_arena,
                         VertexLayoutRef layout);

        // ---- Deferred GPU deletion ----------------------------------------

        // Call once per frame with the id of the frame about to be built /
        // submitted; retired resources are tagged with it.
        void begin_frame(uint64_t frame);

        // Frees everything retired in a frame the GPU has finished with. In the
        // single-threaded case `completed_frame` can simply be the just-drawn
        // frame; with a render thread it is whatever the latest passed fence
        // reports. Dropping the BufferHandle here is what deletes the GL buffer.
        void collect_garbage(uint64_t completed_frame);

    private:
        struct ArenaSlot
        {
            std::optional<BufferArena> arena; // empty when the slot is free
            uint32_t generation = 1;          // 0 is reserved for the null ref
        };

        struct RetiredBuffer
        {
            BufferHandle buffer;
            uint64_t frame; // frame in which it was retired
        };

        struct VaoKey
        {
            std::vector<BufferArenaRef> vbo_arenas;
            BufferArenaRef ebo_arena;
            VertexLayoutRef layout;

            bool operator==(const VaoKey&) const = default;
        };

        struct VaoEntry
        {
            VaoKey key;
            VertexArrayHandle vao; // owned here
        };

        uint32_t acquire_arena_slot();

        void retire(BufferHandle buffer);

        // Re-points every cached VAO that binds `ref`'s arena at the arena's
        // new buffer id after a grow. The VAO id is left unchanged — only the
        // baked-in buffer object is swapped — so meshes holding the id stay
        // valid and nothing in the snapshot needs patching.
        void rebuild_vaos_for_arena(BufferArenaRef ref);

        uint32_t build_vao(VaoKey key);

        static bool key_references(const VaoKey& key, BufferArenaRef ref);

        std::vector<ArenaSlot> arenas_;
        std::vector<uint32_t> free_arena_slots_;
        std::vector<RetiredBuffer> retired_buffers_;
        std::vector<VaoEntry> vaos_;
        uint64_t current_frame_ = 0;
    };
}
