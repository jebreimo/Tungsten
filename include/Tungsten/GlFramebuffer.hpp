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
#include "Types.hpp"

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
                                GLenum tex_target,
                                uint32_t texture,
                                int32_t level = 0);

    [[nodiscard]]
    GLenum check_framebuffer_status(FramebufferTarget target);

    void assert_framebuffer_complete(FramebufferTarget target);
}
