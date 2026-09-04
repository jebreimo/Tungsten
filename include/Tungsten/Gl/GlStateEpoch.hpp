//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>

namespace Tungsten
{
    /**
     * Signals that GL binding state has been changed outside a state cache —
     * by a subsystem that binds directly (VaoCache while baking a VAO,
     * ShaderLibrary while pointing a new variant's samplers at their units),
     * by another event loop drawing over the same context, or by a context
     * reset. Every cache notices at its next call and forgets what it believed
     * was bound.
     *
     * A free counter rather than a call on a particular cache, because the code
     * that binds behind the caches' backs has no route to them — and one GL
     * context may have several caches over it, all of which have to be told.
     * Not thread-safe, which matches the single-threaded GL context the design
     * assumes.
     *
     * This lives in the Gl layer rather than beside GlStateCache because both
     * the layer above it (Neo) and the application scaffold (SdlApplication,
     * which announces the boundary between two event loops' draws) have to
     * reach it, and the scaffold must not depend on Neo.
     */
    void notify_gl_state_changed();

    /**
     * Returns the current value of the counter notify_gl_state_changed() bumps.
     */
    [[nodiscard]]
    uint64_t gl_state_epoch();
} // Tungsten
