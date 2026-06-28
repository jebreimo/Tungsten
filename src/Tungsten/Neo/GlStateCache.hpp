//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <unordered_map>

namespace Tungsten
{
    // Elides redundant GL binding calls by remembering what is currently bound
    // and skipping a bind whose target is already current. The Renderer routes
    // its program / VAO / texture binds through here so a sort-key-batched draw
    // list pays for a state change only when the state actually changes.
    //
    // The cache tracks raw GL ids, never owning GlHandle types: the bound
    // objects are owned by ResourceManager, and the cache only observes which id
    // is current. It must therefore never outlive, delete, or be the sole
    // reference to anything it names. A zero id means "nothing bound", matching
    // the GL default state a fresh cache assumes.
    //
    // The cache is only correct as long as every bind of these targets goes
    // through it. If GL state is changed behind its back (another subsystem,
    // a context reset), call invalidate() so the next bind is not wrongly
    // skipped.
    class GlStateCache
    {
    public:
        void use_program(uint32_t program_id);

        void bind_vao(uint32_t vao_id);

        // Binds a 2D texture to the given texture unit (the sampler slot).
        void bind_texture(int32_t unit, uint32_t texture_id);

        // Binds a material's uniform buffer to the per-material binding point.
        // This is the bind a sort-key-batched draw list repeats most, so it is
        // the one the cache exists to elide: consecutive items sharing a
        // material rebind nothing.
        void bind_material_ubo(uint32_t buffer_id);

        // Forgets all cached bindings, forcing the next bind of each target to
        // issue its GL call. Use after GL state is changed outside the cache.
        void invalidate();

    private:
        uint32_t current_program_ = 0;
        uint32_t current_vao_ = 0;
        uint32_t current_material_ubo_ = 0;
        std::unordered_map<int32_t, uint32_t> bound_textures_;
    };
} // Tungsten