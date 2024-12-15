//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlFrameBuffers.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlFrameBufferDeleter::operator()(GLuint id) const
    {
        glDeleteFramebuffers(1, &id);
    }

    FrameBufferHandle generate_frame_buffer()
    {
        GLuint id;
        glGenFramebuffers(1, &id);
        THROW_IF_GL_ERROR();
        return FrameBufferHandle(id);
    }

    std::vector<FrameBufferHandle> generate_frame_buffers(GLsizei count)
    {
        if (count == 0)
            return {};

        auto ids = std::vector<GLuint>(size_t(count));
        glGenFramebuffers(count, ids.data());
        THROW_IF_GL_ERROR();
        auto result = std::vector<FrameBufferHandle>();
        for (auto id : ids)
            result.emplace_back(id);
        return result;
    }

    void generate_frame_buffers(std::span<FrameBufferHandle> frame_buffers)
    {
        auto ids = std::vector<GLuint>(size_t(frame_buffers.size()));
        glGenFramebuffers(GLsizei(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            frame_buffers[i] = FrameBufferHandle(ids[i]);
    }

    void bind_frame_buffer(GLenum target, GLuint frame_buffer)
    {
        glBindFramebuffer(target, frame_buffer);
        THROW_IF_GL_ERROR();
    }

    void frame_buffer_texture_2d(GLenum target,
                                 GLenum attachment,
                                 GLenum tex_target,
                                 GLuint texture,
                                 GLint level)
    {
        glFramebufferTexture2D(target, attachment, tex_target, texture, level);
        THROW_IF_GL_ERROR();
    }

    FrameBufferTarget::FrameBufferTarget(GLenum target, GLuint frame_buffer)
        : target_(target)
    {
        bind_frame_buffer(target, frame_buffer);
    }

    void FrameBufferTarget::texture_2d(GLenum attachment,
                                       GLenum tex_target,
                                       GLuint texture,
                                       GLint level) const
    {
        frame_buffer_texture_2d(target_, attachment, tex_target, texture, level);
    }

    GLenum FrameBufferTarget::check_status() const
    {
        auto result = glCheckFramebufferStatus(target_);
        THROW_IF_GL_ERROR();
        return result;
    }

    void FrameBufferTarget::assert_complete() const
    {
        if (check_status() != GL_FRAMEBUFFER_COMPLETE)
            throw TungstenException("Frame buffer is incomplete.");
    }
}
