//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/VaoCache.hpp"
#include <algorithm>
#include <utility>
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Neo/BufferArena.hpp"
#include "Tungsten/Neo/DeletionQueue.hpp"
#include "Tungsten/Neo/VertexLayout.hpp"

namespace Tungsten
{
    VaoCache::VaoCache(ArenaResolver arenas, LayoutResolver layouts)
        : arenas_(std::move(arenas)),
          layouts_(std::move(layouts))
    {}

    uint32_t VaoCache::get_vao(std::span<const BufferArenaRef> vbo_arenas,
                               BufferArenaRef ebo_arena,
                               VertexLayoutRef layout)
    {
        VaoKey key{{vbo_arenas.begin(), vbo_arenas.end()}, ebo_arena, layout};
        for (const auto& entry : entries_)
        {
            if (entry.key == key)
                return entry.vao.id();
        }
        return build_vao(std::move(key));
    }

    uint32_t VaoCache::build_vao(VaoKey key)
    {
        VertexArrayHandle vao = generate_vertex_array();
        const uint32_t id = vao.id();

        bind_vertex_array(id);

        const VertexLayout& layout = layouts_(key.layout);
        for (size_t stream = 0; stream < key.vbo_arenas.size(); ++stream)
            bind_stream(key, layout, stream);

        // The element-array binding is VAO state, which is why a VAO is specific
        // to one EBO arena (§13).
        bind_buffer(BufferTarget::ELEMENT_ARRAY,
                    arenas_(key.ebo_arena).buffer_id());

        bind_vertex_array(0);

        entries_.push_back({std::move(key), std::move(vao)});
        return id;
    }

    void VaoCache::bind_stream(const VaoKey& key, const VertexLayout& layout,
                               size_t stream)
    {
        const BufferArena& arena = arenas_(key.vbo_arenas[stream]);

        // GLES 3.00 has no separate vertex-buffer binding points, so an attribute
        // reads from whatever buffer is bound to ARRAY when its pointer is
        // defined (§13). Re-pointing on a grow re-runs exactly this.
        bind_buffer(BufferTarget::ARRAY, arena.buffer_id());

        // One arena per stride (§7), so the arena is the authority on the stream's
        // vertex pitch — not layout.stride or VertexStream::stride.
        const int32_t stride = arena.stride();
        for (const VertexAttribute& attr : layout.attributes)
        {
            if (attr.stream_index != stream)
                continue;

            const uint32_t location = attribute_location(attr.semantic);
            define_vertex_attribute_pointer(location, attr.component_count,
                                            attr.data_type, stride,
                                            attr.offset_in_stream,
                                            attr.normalized);
            enable_vertex_attribute(location);
        }
    }

    void VaoCache::rebuild_for_arena(BufferArenaRef ref)
    {
        for (auto& entry : entries_)
        {
            if (!key_references(entry.key, ref))
                continue;

            bind_vertex_array(entry.vao.id());

            const VertexLayout& layout = layouts_(entry.key.layout);
            for (size_t stream = 0; stream < entry.key.vbo_arenas.size(); ++stream)
            {
                if (entry.key.vbo_arenas[stream] == ref)
                    bind_stream(entry.key, layout, stream);
            }

            if (entry.key.ebo_arena == ref)
                bind_buffer(BufferTarget::ELEMENT_ARRAY, arenas_(ref).buffer_id());
        }
        bind_vertex_array(0);
    }

    void VaoCache::evict_for_arena(BufferArenaRef ref, DeletionQueue& deletions)
    {
        for (auto& entry : entries_)
        {
            if (key_references(entry.key, ref))
                deletions.retire(std::move(entry.vao));
        }
        std::erase_if(entries_, [&](const VaoEntry& entry) {
            return key_references(entry.key, ref);
        });
    }

    bool VaoCache::key_references(const VaoKey& key, BufferArenaRef ref)
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
} // Tungsten