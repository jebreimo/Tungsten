//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlFrameBufferDeleter
    {
        void operator()(GLuint id) const;
    };

    using FrameBufferHandle = GlHandle<GlFrameBufferDeleter>;

    FrameBufferHandle generate_frame_buffer();

    std::vector<FrameBufferHandle> generate_frame_buffers(GLsizei count);

    void generate_frame_buffers(std::span<FrameBufferHandle> frame_buffers);

    void bind_frame_buffer(GLenum target, GLuint frame_buffer);

    void frame_buffer_texture_2d(GLenum target,
                                 GLenum attachment,
                                 GLenum tex_target,
                                 GLuint texture,
                                 GLint level);

    class FrameBufferTarget
    {
    public:
        FrameBufferTarget(GLenum target, GLuint frame_buffer);

        void texture_2d(GLenum attachment, GLenum tex_target, GLuint texture,
                        GLint level = 0) const;

        [[nodiscard]]
        GLenum check_status() const;

        void assert_complete() const;
    private:
        GLenum target_;
    };
}
