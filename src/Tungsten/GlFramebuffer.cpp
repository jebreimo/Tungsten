//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlFramebuffer.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void FramebufferDeleter::operator()(GLuint id) const
    {
        glDeleteFramebuffers(1, &id);
    }

    FramebufferHandle generate_framebuffer()
    {
        GLuint id;
        glGenFramebuffers(1, &id);
        THROW_IF_GL_ERROR();
        return FramebufferHandle(id);
    }

    void generates(std::span<FramebufferHandle> frame_buffers)
    {
        auto ids = std::vector<GLuint>(frame_buffers.size());
        glGenFramebuffers(static_cast<GLsizei>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            frame_buffers[i] = FramebufferHandle(ids[i]);
    }

    void bind_framebuffer(GLenum target, GLuint frame_buffer)
    {
        glBindFramebuffer(target, frame_buffer);
        THROW_IF_GL_ERROR();
    }

    void framebuffer_texture_2d(GLenum target,
                                GLenum attachment,
                                GLenum tex_target,
                                GLuint texture,
                                GLint level)
    {
        glFramebufferTexture2D(target, attachment, tex_target, texture, level);
        THROW_IF_GL_ERROR();
    }

    GLenum check_framebuffer_status(GLenum target)
    {
        auto result = glCheckFramebufferStatus(target);
        THROW_IF_GL_ERROR();
        return result;
    }

    void assert_framebuffer_complete(GLenum target)
    {
        if (check_framebuffer_status(target) != GL_FRAMEBUFFER_COMPLETE)
            TUNGSTEN_THROW("Frame buffer is incomplete.");
    }
}
