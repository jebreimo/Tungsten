//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlFramebuffer.hpp"

#include <vector>

#include "GlTypeConversion.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void FramebufferDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().deleteFramebuffers(1, &id);
    }

    FramebufferHandle generate_framebuffer()
    {
        uint32_t id;
        get_ogl_wrapper().genFramebuffers(1, &id);
        THROW_IF_GL_ERROR();
        return FramebufferHandle(id);
    }

    void generates(std::span<FramebufferHandle> frame_buffers)
    {
        auto ids = std::vector<uint32_t>(frame_buffers.size());
        get_ogl_wrapper().genFramebuffers(static_cast<int32_t>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            frame_buffers[i] = FramebufferHandle(ids[i]);
    }

    void bind_framebuffer(FramebufferTarget target, uint32_t frame_buffer)
    {
        get_ogl_wrapper().bindFramebuffer(to_ogl_framebuffer_target(target), frame_buffer);
        THROW_IF_GL_ERROR();
    }

    void framebuffer_texture_2d(FramebufferTarget target,
                                FrameBufferAttachment attachment,
                                TextureTarget2D tex_target,
                                uint32_t texture,
                                int32_t level)
    {
        get_ogl_wrapper().framebufferTexture2D(to_ogl_framebuffer_target(target),
                               to_ogl_framebuffer_attachment(attachment),
                               to_ogl_texture_target_2d(tex_target), texture, level);
        THROW_IF_GL_ERROR();
    }

    GLenum check_framebuffer_status(FramebufferTarget target)
    {
        auto result = get_ogl_wrapper().checkFramebufferStatus(to_ogl_framebuffer_target(target));
        THROW_IF_GL_ERROR();
        return result;
    }

    void assert_framebuffer_complete(FramebufferTarget target)
    {
        if (check_framebuffer_status(target) != GL_FRAMEBUFFER_COMPLETE)
            TUNGSTEN_THROW("Frame buffer is incomplete.");
    }
}
