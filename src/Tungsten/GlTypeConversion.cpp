//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GlTypeConversion.hpp"

#include "Tungsten/Environment.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    GLenum to_ogl_buffer_target(BufferTarget target)
    {
        switch (target)
        {
        case BufferTarget::ARRAY: return GL_ARRAY_BUFFER;
        case BufferTarget::ELEMENT_ARRAY: return GL_ELEMENT_ARRAY_BUFFER;
        default:
            TUNGSTEN_THROW("Unsupported buffer target: " + std::to_string(static_cast<int>(target)));
        }
    }

    GLenum to_ogl_buffer_usage(BufferUsage usage)
    {
        switch (usage)
        {
        case BufferUsage::STATIC_DRAW: return GL_STATIC_DRAW;
        case BufferUsage::DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
        default:
            TUNGSTEN_THROW("Unsupported buffer usage: " + std::to_string(static_cast<int>(usage)));
        }
    }

    BufferUsage from_ogl_buffer_usage(GLenum usage)
    {
        switch (usage)
        {
        case GL_STATIC_DRAW: return BufferUsage::STATIC_DRAW;
        case GL_DYNAMIC_DRAW: return BufferUsage::DYNAMIC_DRAW;
        default:
            TUNGSTEN_THROW("Unsupported buffer usage: " + std::to_string(static_cast<int>(usage)));
        }
    }

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

    GLenum to_ogl_texture_target(TextureTarget target)
    {
        switch (target)
        {
        case TextureTarget::TEXTURE_2D: return GL_TEXTURE_2D;
        case TextureTarget::CUBE_MAP: return GL_TEXTURE_CUBE_MAP;
        case TextureTarget::TEXTURE_3D: return GL_TEXTURE_3D;
        case TextureTarget::ARRAY_2D: return GL_TEXTURE_2D_ARRAY;
        default: TUNGSTEN_THROW("Unsupported texture target: " + std::to_string(static_cast<int>(target)));
        }
    }

    GLenum to_ogl_texture_target_2d(TextureTarget2D target)
    {
        switch (target)
        {
        case TextureTarget2D::TEXTURE_2D: return GL_TEXTURE_2D;
        case TextureTarget2D::CUBE_MAP_POSITIVE_X: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case TextureTarget2D::CUBE_MAP_POSITIVE_Y: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case TextureTarget2D::CUBE_MAP_POSITIVE_Z: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case TextureTarget2D::CUBE_MAP_NEGATIVE_X: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case TextureTarget2D::CUBE_MAP_NEGATIVE_Y: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case TextureTarget2D::CUBE_MAP_NEGATIVE_Z: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default: TUNGSTEN_THROW("Unsupported texture target: " + std::to_string(static_cast<int>(target)));
        }
    }

    GLenum to_ogl_texture_format(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::R:
            if constexpr (is_emscripten())
                return GL_LUMINANCE;
            else
                return GL_RED;
        case TextureFormat::RGB:
            return GL_RGB;
        case TextureFormat::RGBA:
            return GL_RGBA;
        default:
            TUNGSTEN_THROW("Unsupported texture format: " + std::to_string(static_cast<int>(format)));
        }
    }

    GLenum to_ogl_texture_value_type(TextureValueType type)
    {
        switch (type)
        {
        case TextureValueType::UINT8:
            return GL_UNSIGNED_BYTE;
        case TextureValueType::FLOAT:
            return GL_FLOAT;
        default:
            TUNGSTEN_THROW("Unsupported texture type: " + std::to_string(static_cast<int>(type)));
        }
    }

    GLenum to_ogl_texture_parameter(TextureParameter parameter)
    {
        switch (parameter)
        {
        case TextureParameter::BASE_LEVEL: return GL_TEXTURE_BASE_LEVEL;
        case TextureParameter::MAX_LEVEL: return GL_TEXTURE_MAX_LEVEL;
        case TextureParameter::MIN_FILTER: return GL_TEXTURE_MIN_FILTER;
        case TextureParameter::MAG_FILTER: return GL_TEXTURE_MAG_FILTER;
        case TextureParameter::WRAP_S: return GL_TEXTURE_WRAP_S;
        case TextureParameter::WRAP_T: return GL_TEXTURE_WRAP_T;
        case TextureParameter::WRAP_R: return GL_TEXTURE_WRAP_R;
        case TextureParameter::COMPARE_MODE: return GL_TEXTURE_COMPARE_MODE;
        case TextureParameter::COMPARE_FUNC: return GL_TEXTURE_COMPARE_FUNC;
        case TextureParameter::MIN_LOD: return GL_TEXTURE_MIN_LOD;
        case TextureParameter::MAX_LOD: return GL_TEXTURE_MAX_LOD;
        case TextureParameter::SWIZZLE_R: return GL_TEXTURE_SWIZZLE_R;
        case TextureParameter::SWIZZLE_G: return GL_TEXTURE_SWIZZLE_G;
        case TextureParameter::SWIZZLE_B: return GL_TEXTURE_SWIZZLE_B;
        case TextureParameter::SWIZZLE_A: return GL_TEXTURE_SWIZZLE_A;
        default: TUNGSTEN_THROW("Unsupported texture parameter: " + std::to_string(static_cast<int>(parameter)));
        }
    }

    GLenum to_ogl_draw_mode(TopologyType topology)
    {
        switch (topology)
        {
        case TopologyType::POINTS: return GL_POINTS;
        case TopologyType::LINES: return GL_LINES;
        case TopologyType::LINE_STRIP: return GL_LINE_STRIP;
        case TopologyType::LINE_LOOP: return GL_LINE_LOOP;
        case TopologyType::TRIANGLES: return GL_TRIANGLES;
        case TopologyType::TRIANGLE_FAN: return GL_TRIANGLE_FAN;
        case TopologyType::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
        default: TUNGSTEN_THROW("Unsupported draw mode: " + std::to_string(static_cast<int>(topology)));
        }
    }

    GLenum to_ogl_shader_type(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::VERTEX:
            return GL_VERTEX_SHADER;
        case ShaderType::FRAGMENT:
            return GL_FRAGMENT_SHADER;
        case ShaderType::COMPUTE:
            return GL_COMPUTE_SHADER;
        default:
            TUNGSTEN_THROW("Unknown ShaderType: "
                + std::to_string(static_cast<int>(type)));
        }
    }
}
