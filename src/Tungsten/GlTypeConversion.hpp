//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>
#include "Tungsten/GlTypes.hpp"

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
    GLenum to_ogl_texture_parameter(TextureParameter parameter);
    GLenum to_ogl_draw_mode(TopologyType topology);
    GLenum to_ogl_shader_type(ShaderType type);
    GLenum to_ogl_vertex_attribute_type(VertexAttributeType type);
    FramebufferStatus from_ogl_framebuffer_status(GLenum status);
    GLenum to_ogl_element_index_type(ElementIndexType type);
    GLint to_ogl_texture_min_filter(TextureMinFilter filter);
    TextureMinFilter from_ogl_texture_min_filter(GLint filter);
    GLint to_ogl_texture_mag_filter(TextureMagFilter filter);
    TextureMagFilter from_ogl_texture_mag_filter(GLint filter);
    GLint to_ogl_texture_wrap_mode(TextureWrapMode mode);
    TextureWrapMode from_ogl_texture_wrap_mode(GLint mode);
    GLenum to_ogl_blend_function(BlendFunction blend_function);
    GLenum to_ogl_cull_mode(CullMode mode);
}
