//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>

namespace Tungsten
{
    /**
     * Signals that GL binding state has been changed outside a GlStateCache —
     * by a subsystem that binds directly (VaoCache while baking a VAO,
     * ShaderLibrary while pointing a new variant's samplers at their units),
     * or by a context reset. Every cache notices at its next call and forgets
     * what it believed was bound.
     *
     * A free counter rather than a call on a particular cache, because the
     * code that binds behind the caches' backs lives under ResourceManager and
     * has no route to a Renderer's cache — and one GL context may have several
     * caches over it, all of which have to be told. Not thread-safe, which
     * matches the single-threaded GL context the design assumes.
     */
    void notify_gl_state_changed();

    /**
     * Returns the current value of the counter notify_gl_state_changed() bumps.
     */
    [[nodiscard]]
    uint64_t gl_state_epoch();

    /**
     * Elides redundant GL binding calls by remembering what is currently bound
     * and skipping a bind whose target is already current. The Renderer routes
     * its program / VAO / texture binds through here so a sort-key-batched draw
     * list pays for a state change only when the state actually changes.
     *
     * The cache tracks raw GL ids, never owning GlHandle types: the bound
     * objects are owned by ResourceManager, and the cache only observes which id
     * is current. It must therefore never outlive, delete, or be the sole
     * reference to anything it names.
     *
     * The cache is only correct as long as every bind of these targets either
     * goes through it or is announced with notify_gl_state_changed(). Nothing
     * is assumed about the state of a fresh cache: each target is "unknown"
     * until this cache binds it, so the first bind of each always issues.
     */
    class GlStateCache
    {
    public:
        void use_program(uint32_t program_id);

        void bind_vao(uint32_t vao_id);

        /**
         * Binds a 2D texture to the given texture unit (the sampler slot).
         * @param unit The texture unit to bind to, e.g. 0 for GL_TEXTURE0.
         * @param texture_id The GL id of the texture to bind, or 0 to unbind.
         */
        void bind_texture(int32_t unit, uint32_t texture_id);

        /**
         * Binds a material's own parameter buffer to the per-material binding
         * point. This is the bind a sort-key-batched draw list repeats most,
         * so it is the one the cache exists to elide: consecutive items
         * sharing a material rebind nothing.
         */
        void bind_material_ubo(uint32_t buffer_id);

        /**
         * Forgets all cached bindings, forcing the next bind of each target to
         * issue its GL call. Called automatically when the epoch moves; call
         * it directly only to force the issue.
         */
        void invalidate();

    private:
        /**
         * Drops everything if someone bound outside this cache since the last
         * call. Every public method runs this first.
         */
        void sync();

        /**
         * Empty means "unknown", which never compares equal to an id — so an
         * invalidated cache re-issues even a bind of 0.
         */
        std::optional<uint32_t> current_program_;
        std::optional<uint32_t> current_vao_;
        std::optional<uint32_t> current_material_ubo_;
        /**
         * Maps texture units to the currently bound texture id.
         */
        std::unordered_map<int32_t, uint32_t> bound_textures_;
        uint64_t epoch_seen_ = gl_state_epoch();
    };
} // Tungsten
