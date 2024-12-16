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

namespace Tungsten
{
    struct FramebufferDeleter
    {
        void operator()(GLuint id) const;
    };

    using FramebufferHandle = GlHandle<FramebufferDeleter>;

    FramebufferHandle generate_framebuffer();

    void generate_framebuffers(std::span<FramebufferHandle> frame_buffers);

    void bind_framebuffer(GLenum target, GLuint frame_buffer);

    GLuint bound_framebuffer(GLenum target);

    void framebuffer_texture_2d(GLenum target,
                                GLenum attachment,
                                GLenum tex_target,
                                GLuint texture,
                                GLint level = 0);

    [[nodiscard]]
    GLenum check_framebuffer_status(GLenum target);

    void assert_framebuffer_complete(GLenum target);
}
