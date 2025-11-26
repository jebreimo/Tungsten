//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlFramebuffer.hpp"

#include <vector>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    GLenum to_ogl_framebuffer_target(FramebufferTarget target)
    {
        switch (target)
        {
        case FramebufferTarget::FRAMEBUFFER: return GL_FRAMEBUFFER;
        case FramebufferTarget::DRAW: return GL_DRAW_FRAMEBUFFER;
        case FramebufferTarget::READ: return GL_READ_FRAMEBUFFER;
        default:
            TUNGSTEN_THROW("Unsupported framebuffer target: " + std::to_string(static_cast<int>(target)));
        }
    }

    GLenum to_ogl_framebuffer_attachment(FrameBufferAttachment attachment)
    {
        switch (attachment)
        {
        case FrameBufferAttachment::COLOR0: return GL_COLOR_ATTACHMENT0;
        case FrameBufferAttachment::DEPTH: return GL_DEPTH_ATTACHMENT;
        case FrameBufferAttachment::STENCIL: return GL_STENCIL_ATTACHMENT;
        default: TUNGSTEN_THROW("Unsupported framebuffer attachment: " + std::to_string(static_cast<int>(attachment)));
        }
    }

    void FramebufferDeleter::operator()(uint32_t id) const
    {
        glDeleteFramebuffers(1, &id);
    }

    FramebufferHandle generate_framebuffer()
    {
        uint32_t id;
        glGenFramebuffers(1, &id);
        THROW_IF_GL_ERROR();
        return FramebufferHandle(id);
    }

    void generates(std::span<FramebufferHandle> frame_buffers)
    {
        auto ids = std::vector<uint32_t>(frame_buffers.size());
        glGenFramebuffers(static_cast<int32_t>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            frame_buffers[i] = FramebufferHandle(ids[i]);
    }

    void bind_framebuffer(FramebufferTarget target, uint32_t frame_buffer)
    {
        glBindFramebuffer(to_ogl_framebuffer_target(target), frame_buffer);
        THROW_IF_GL_ERROR();
    }

    void framebuffer_texture_2d(FramebufferTarget target,
                                FrameBufferAttachment attachment,
                                GLenum tex_target,
                                uint32_t texture,
                                int32_t level)
    {
        glFramebufferTexture2D(to_ogl_framebuffer_target(target),
                               to_ogl_framebuffer_attachment(attachment),
                               tex_target, texture, level);
        THROW_IF_GL_ERROR();
    }

    GLenum check_framebuffer_status(FramebufferTarget target)
    {
        auto result = glCheckFramebufferStatus(to_ogl_framebuffer_target(target));
        THROW_IF_GL_ERROR();
        return result;
    }

    void assert_framebuffer_complete(FramebufferTarget target)
    {
        if (check_framebuffer_status(target) != GL_FRAMEBUFFER_COMPLETE)
            TUNGSTEN_THROW("Frame buffer is incomplete.");
    }
}
