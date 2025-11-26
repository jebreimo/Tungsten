//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Types.hpp"

namespace Tungsten
{
    GLenum to_ogl_buffer_target(BufferTarget target);
    GLenum to_ogl_buffer_usage(BufferUsage usage);
    BufferUsage from_ogl_buffer_usage(GLenum usage);
    GLenum to_ogl_framebuffer_target(FramebufferTarget target);
    GLenum to_ogl_framebuffer_attachment(FrameBufferAttachment attachment);
    GLenum to_ogl_texture_target(TextureTarget target);
    GLenum to_ogl_texture_target_2d(TextureTarget2D target);
    GLenum to_ogl_texture_format(TextureFormat format);
    GLenum to_ogl_texture_value_type(TextureValueType type);
}
