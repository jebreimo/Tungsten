//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <functional>
#include <span>
#include <vector>
#include "ResourceRefs.hpp"
#include "Tungsten/Gl/GlVertexArray.hpp"

namespace Tungsten
{
    class BufferArena;
    class DeletionQueue;

    // The shared-VAO cache (§13), owned by ResourceManager. A VAO bakes in its
    // buffer-object bindings (each vertex stream's VBO and the element buffer)
    // and the attribute format, so it is valid for exactly one
    // (vbo arenas, ebo arena, layout) combination and is shared by every mesh
    // with that combination regardless of which shader draws it — attribute
    // locations follow the fixed AttributeSemantic convention, not the shader.
    // Per-mesh base offsets (first_vertex, first_index) are NOT baked, so meshes
    // differing only by offset reuse one VAO.
    //
    // The cache owns each VAO (returning a non-owning id, not a handle): since
    // VAOs are shared, no single Mesh can own one. It does not own the arenas or
    // layouts it bakes; it resolves them through the injected resolvers (the arena
    // pool and LayoutRegistry), and retires evicted VAOs through the DeletionQueue.
    // The key holds arena *refs*, so an entry survives an arena grow — see
    // rebuild_for_arena.
    class VaoCache
    {
    public:
        using ArenaResolver = std::function<const BufferArena&(BufferArenaRef)>;
        using LayoutResolver = std::function<const VertexLayout&(VertexLayoutRef)>;

        VaoCache(ArenaResolver arenas, LayoutResolver layouts);

        // Returns the VAO for the given arenas + layout, building and caching it
        // on first use.
        uint32_t get_vao(std::span<const BufferArenaRef> vbo_arenas,
                         BufferArenaRef ebo_arena,
                         VertexLayoutRef layout);

        // Re-points every cached VAO that binds `ref`'s arena at the arena's new
        // buffer id after a grow. The VAO id is left unchanged — only the baked-in
        // buffer object is swapped — so meshes holding the id stay valid and
        // nothing in the snapshot needs patching.
        void rebuild_for_arena(BufferArenaRef ref);

        // Removes every cached VAO that binds `ref`'s arena, retiring its handle
        // through the deferred queue. Called when an arena is destroyed, which is
        // only valid once no live mesh draws from it.
        void evict_for_arena(BufferArenaRef ref, DeletionQueue& deletions);

    private:
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

        uint32_t build_vao(VaoKey key);

        // Binds one vertex stream's buffer and defines the attributes read from
        // it. Assumes the target VAO is already bound.
        void bind_stream(const VaoKey& key, const VertexLayout& layout,
                         size_t stream);

        static bool key_references(const VaoKey& key, BufferArenaRef ref);

        ArenaResolver arenas_;
        LayoutResolver layouts_;
        std::vector<VaoEntry> entries_;
    };
} // Tungsten