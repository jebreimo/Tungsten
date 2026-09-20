//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include "GlHandle.hpp"
#include "../Gpu/GpuTypes.hpp"

namespace Tungsten
{
    struct FramebufferDeleter
    {
        void operator()(uint32_t id) const;
    };

    using FramebufferHandle = GlHandle<FramebufferDeleter>;

    FramebufferHandle generate_framebuffer();

    void generate_framebuffers(std::span<FramebufferHandle> frame_buffers);

    void bind_framebuffer(FramebufferTarget target, uint32_t frame_buffer);

    uint32_t bound_framebuffer(FramebufferTarget target);

    void framebuffer_texture_2d(FramebufferTarget target,
                                FrameBufferAttachment attachment,
                                TextureTarget2D tex_target,
                                uint32_t texture,
                                int32_t level = 0);

    /**
     * Tells the GL that the given attachments' contents are no longer needed,
     * so a tiled renderer can skip writing them back (or loading them).
     *
     * The default framebuffer names its attachments differently from one of
     * our own, which is what @a is_default_framebuffer selects.
     */
    void invalidate_framebuffer(
        FramebufferTarget target,
        std::span<const FrameBufferAttachment> attachments,
        bool is_default_framebuffer);

    [[nodiscard]]
    FramebufferStatus check_framebuffer_status(FramebufferTarget target);

    void assert_framebuffer_complete(FramebufferTarget target);
}
