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
            TUNGSTEN_THROW(
                "Unsupported buffer target: " + std::to_string(static_cast<int>(target)));
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
            TUNGSTEN_THROW(
                "Unsupported framebuffer target: " + std::to_string(static_cast<int>(target)));
        }
    }

    GLenum to_ogl_framebuffer_attachment(FrameBufferAttachment attachment)
    {
        switch (attachment)
        {
        case FrameBufferAttachment::COLOR0: return GL_COLOR_ATTACHMENT0;
        case FrameBufferAttachment::DEPTH: return GL_DEPTH_ATTACHMENT;
        case FrameBufferAttachment::STENCIL: return GL_STENCIL_ATTACHMENT;
        default: TUNGSTEN_THROW(
                "Unsupported framebuffer attachment: " + std::to_string(static_cast<int>(attachment)
                ));
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
        default: TUNGSTEN_THROW(
                "Unsupported texture target: " + std::to_string(static_cast<int>(target)));
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
        default: TUNGSTEN_THROW(
                "Unsupported texture target: " + std::to_string(static_cast<int>(target)));
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
            TUNGSTEN_THROW(
                "Unsupported texture format: " + std::to_string(static_cast<int>(format)));
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
        default: TUNGSTEN_THROW(
                "Unsupported texture parameter: " + std::to_string(static_cast<int>(parameter)));
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
        default: TUNGSTEN_THROW(
                "Unsupported draw mode: " + std::to_string(static_cast<int>(topology)));
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

    GLenum to_ogl_vertex_attribute_type(VertexAttributeType type)
    {
        switch (type)
        {
        case VertexAttributeType::INT8:
            return GL_BYTE;
        case VertexAttributeType::UINT8:
            return GL_UNSIGNED_BYTE;
        case VertexAttributeType::INT16:
            return GL_SHORT;
        case VertexAttributeType::UINT16:
            return GL_UNSIGNED_SHORT;
        case VertexAttributeType::INT32:
            return GL_INT;
        case VertexAttributeType::UINT32:
            return GL_UNSIGNED_INT;
        case VertexAttributeType::FLOAT:
            return GL_FLOAT;
        case VertexAttributeType::DOUBLE:
            return GL_DOUBLE;
        default:
            TUNGSTEN_THROW("Unknown VertexAttributeType: "
                + std::to_string(static_cast<int>(type)));
        }
    }

    FramebufferStatus from_ogl_framebuffer_status(GLenum status)
    {
        switch (status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            return FramebufferStatus::COMPLETE;
        case GL_FRAMEBUFFER_UNDEFINED:
            return FramebufferStatus::UNDEFINED;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return FramebufferStatus::INCOMPLETE_ATTACHMENT;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return FramebufferStatus::INCOMPLETE_MISSING_ATTACHMENT;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return FramebufferStatus::UNSUPPORTED;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return FramebufferStatus::INCOMPLETE_MULTISAMPLE;
        default:
            TUNGSTEN_THROW("Unknown FramebufferStatus: "
                + std::to_string(static_cast<int>(status)));
        }
    }

    GLenum to_ogl_element_index_type(ElementIndexType type)
    {
        switch (type)
        {
        case ElementIndexType::UINT8:
            return GL_UNSIGNED_BYTE;
        case ElementIndexType::UINT16:
            return GL_UNSIGNED_SHORT;
        case ElementIndexType::UINT32:
            return GL_UNSIGNED_INT;
        default:
            TUNGSTEN_THROW("Unknown ElementIndexType: "
                + std::to_string(static_cast<int>(type)));
        }
    }

    GLint to_ogl_texture_min_filter(TextureMinFilter filter)
    {
        switch (filter)
        {
        case TextureMinFilter::NEAREST: return GL_NEAREST;
        case TextureMinFilter::LINEAR: return GL_LINEAR;
        case TextureMinFilter::NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureMinFilter::LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
        case TextureMinFilter::NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
        case TextureMinFilter::LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
        default:
            TUNGSTEN_THROW("Unsupported texture min filter: "
                + std::to_string(static_cast<int>(filter)));
        }
    }

    TextureMinFilter from_ogl_texture_min_filter(GLint filter)
    {
        switch (filter)
        {
        case GL_NEAREST: return TextureMinFilter::NEAREST;
        case GL_LINEAR: return TextureMinFilter::LINEAR;
        case GL_NEAREST_MIPMAP_NEAREST: return TextureMinFilter::NEAREST_MIPMAP_NEAREST;
        case GL_LINEAR_MIPMAP_NEAREST: return TextureMinFilter::LINEAR_MIPMAP_NEAREST;
        case GL_NEAREST_MIPMAP_LINEAR: return TextureMinFilter::NEAREST_MIPMAP_LINEAR;
        case GL_LINEAR_MIPMAP_LINEAR: return TextureMinFilter::LINEAR_MIPMAP_LINEAR;
        default:
            TUNGSTEN_THROW("Unsupported texture min filter: "
                + std::to_string(filter));
        }
    }

    GLint to_ogl_texture_mag_filter(TextureMagFilter filter)
    {
        switch (filter)
        {
        case TextureMagFilter::NEAREST: return GL_NEAREST;
        case TextureMagFilter::LINEAR: return GL_LINEAR;
        default:
            TUNGSTEN_THROW("Unsupported texture mag filter: "
                + std::to_string(static_cast<int>(filter)));
        }
    }

    TextureMagFilter from_ogl_texture_mag_filter(GLint filter)
    {
        switch (filter)
        {
        case GL_NEAREST: return TextureMagFilter::NEAREST;
        case GL_LINEAR: return TextureMagFilter::LINEAR;
        default:
            TUNGSTEN_THROW("Unsupported texture mag filter: "
                + std::to_string(filter));
        }
    }

    GLint to_ogl_texture_wrap_mode(TextureWrapMode mode)
    {
        switch (mode)
        {
        case TextureWrapMode::REPEAT: return GL_REPEAT;
        case TextureWrapMode::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
        case TextureWrapMode::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
        default:
            TUNGSTEN_THROW("Unsupported texture wrap mode: "
                + std::to_string(static_cast<int>(mode)));
        }

    }

    TextureWrapMode from_ogl_texture_wrap_mode(GLint mode)
    {
        switch (mode)
        {
        case GL_REPEAT: return TextureWrapMode::REPEAT;
        case GL_MIRRORED_REPEAT: return TextureWrapMode::MIRRORED_REPEAT;
        case GL_CLAMP_TO_EDGE: return TextureWrapMode::CLAMP_TO_EDGE;
        default:
            TUNGSTEN_THROW("Unsupported texture wrap mode: "
                + std::to_string(mode));
        }
    }

    GLenum to_ogl_blend_function(BlendFunction blend_function)
    {
        switch (blend_function)
        {
        case BlendFunction::ZERO: return GL_ZERO;
        case BlendFunction::ONE: return GL_ONE;
        case BlendFunction::SRC_COLOR: return GL_SRC_COLOR;
        case BlendFunction::ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
        case BlendFunction::DST_COLOR: return GL_DST_COLOR;
        case BlendFunction::ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
        case BlendFunction::SRC_ALPHA: return GL_SRC_ALPHA;
        case BlendFunction::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFunction::DST_ALPHA: return GL_DST_ALPHA;
        case BlendFunction::ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case BlendFunction::CONSTANT_COLOR: return GL_CONSTANT_COLOR;
        case BlendFunction::ONE_MINUS_CONSTANT_COLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFunction::CONSTANT_ALPHA: return GL_CONSTANT_ALPHA;
        case BlendFunction::ONE_MINUS_CONSTANT_ALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFunction::SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
        default:
            TUNGSTEN_THROW("Unsupported blend function: "
                + std::to_string(static_cast<int>(blend_function)));
        }
    }

    GLenum to_ogl_cull_mode(FaceCullingMode mode)
    {
        switch (mode)
        {
        case FaceCullingMode::BACK: return GL_BACK;
        case FaceCullingMode::FRONT: return GL_FRONT;
        case FaceCullingMode::FRONT_AND_BACK: return GL_FRONT_AND_BACK;
        default:
            TUNGSTEN_THROW("Unsupported cull mode: "
                + std::to_string(static_cast<int>(mode)));
        }
    }
}
