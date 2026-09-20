//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <optional>
#include "Tungsten/Gl/GlStateEpoch.hpp"
#include "Tungsten/Resources/PipelineDescriptor.hpp"

namespace Tungsten
{
    /**
     * Applies a PipelineDescriptor to the OpenGL state machine, issuing only
     * the calls that differ from the pipeline bound before it.
     *
     * This sits beside GlStateCache rather than inside it, and the split is
     * deliberate. GlStateCache elides redundant *binds* keyed on GL object
     * names — program, VAO, textures, UBO — and in a Metal or Vulkan backend
     * it disappears entirely, because there is no shared global state to
     * re-bind. This class covers fixed-function state, and in such a backend
     * it collapses the other way: into the single setRenderPipelineState: or
     * vkCmdBindPipeline that the descriptor was designed to become. Keeping
     * them apart means a port deletes one and collapses the other, instead of
     * untangling a class that did both.
     *
     * The cache is only correct while nothing changes this state behind its
     * back, which in a shared context is not a safe assumption: an application
     * may call the Gl/ state functions directly, and two event loops share one
     * context. So it invalidates on the global GL state epoch, and the
     * renderer also invalidates it once per frame, which makes the first draw
     * of every frame emit a complete, deterministic state set.
     */
    class GlPipelineBinder
    {
    public:
        /**
         * Applies @a descriptor, issuing only what differs from the pipeline
         * currently bound. Does not bind the program: that is a GL object
         * name, so it goes through GlStateCache.
         */
        void bind(const PipelineDescriptor& descriptor);

        /**
         * Forgets what is bound, so the next bind() states everything. Called
         * at the start of each frame.
         */
        void invalidate();

    private:
        /** Drops the cache if another subsystem announced a state change. */
        void sync();

        std::optional<PipelineDescriptor> current_;
        uint64_t epoch_seen_ = gl_state_epoch();
    };
} // Tungsten
