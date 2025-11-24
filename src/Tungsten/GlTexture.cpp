//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlTexture.hpp"

#include <vector>
#include "Tungsten/Environment.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        GLenum map_color_format(GLenum format)
        {
            switch (format)
            {
            case GL_RED:
            case GL_R8:
            case GL_LUMINANCE:
                if constexpr (is_emscripten())
                    return GL_LUMINANCE;
                else
                    return GL_RED;
            case GL_RG:
            case GL_RG8:
            case GL_LUMINANCE_ALPHA:
                if constexpr (is_emscripten())
                    return GL_LUMINANCE_ALPHA;
                else
                    return GL_RG;
            case GL_RGB8:
                return GL_RGB;
            case GL_RGBA8:
                return GL_RGBA;
            default:
                return format;
            }
        }

        GLenum map_texture_binding(GLenum target)
        {
            switch (target)
            {
            case GL_TEXTURE_1D:
                return GL_TEXTURE_BINDING_1D;
            case GL_TEXTURE_2D:
                return GL_TEXTURE_BINDING_2D;
            case GL_TEXTURE_3D:
                return GL_TEXTURE_BINDING_3D;
            case GL_TEXTURE_1D_ARRAY:
                return GL_TEXTURE_BINDING_1D_ARRAY;
            case GL_TEXTURE_2D_ARRAY:
                return GL_TEXTURE_BINDING_2D_ARRAY;
            case GL_TEXTURE_RECTANGLE:
                return GL_TEXTURE_BINDING_RECTANGLE;
            case GL_TEXTURE_CUBE_MAP:
                return GL_TEXTURE_BINDING_CUBE_MAP;
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
            case GL_TEXTURE_BUFFER:
                return GL_TEXTURE_BINDING_BUFFER;
            case GL_TEXTURE_2D_MULTISAMPLE:
                return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
            default:
                TUNGSTEN_THROW("Unsupported texture target: " + std::to_string(target));;
            }
        }
    }

    void TextureDeleter::operator()(GLuint id) const
    {
        glDeleteTextures(1, &id);
        THROW_IF_GL_ERROR();
    }

    TextureHandle generate_texture()
    {
        GLuint id;
        glGenTextures(1, &id);
        THROW_IF_GL_ERROR();
        return TextureHandle(id);
    }

    void generate_textures(std::span<TextureHandle> textures)
    {
        auto ids = std::vector<GLuint>(textures.size());
        glGenTextures(static_cast<GLsizei>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            textures[i] = TextureHandle(ids[i]);
    }

    void activate_texture_unit(GLint unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        THROW_IF_GL_ERROR();
    }

    void bind_texture(GLenum target, GLuint texture)
    {
        glBindTexture(target, texture);
        THROW_IF_GL_ERROR();
    }

    GLint active_texture_unit()
    {
        GLint result;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &result);
        THROW_IF_GL_ERROR();
        return result - GL_TEXTURE0;
    }

    GLuint bound_texture(GLenum target)
    {
        GLint result;
        glGetIntegerv(map_texture_binding(target), &result);
        THROW_IF_GL_ERROR();
        return static_cast<GLuint>(result);
    }

    void set_texture_image_2d(GLenum target, GLint level,
                              GLint internal_format,
                              Size2I size,
                              TextureSourceFormat format,
                              const void* data)
    {
        glTexImage2D(target, level,
                     static_cast<GLint>(map_color_format(internal_format)),
                     size.width, size.height, 0,
                     map_color_format(format.format), format.type, data);
        THROW_IF_GL_ERROR();
    }

    void set_texture_storage_2d(GLenum target, GLint levels, GLint internal_format, Size2I size)
    {
        glTexStorage2D(target, levels,
                       static_cast<GLint>(map_color_format(internal_format)),
                       size.width, size.height);
        THROW_IF_GL_ERROR();
    }

    void set_texture_sub_image_2d(GLenum target, GLint level,
                                  Position2I offset,
                                  Size2I size,
                                  TextureSourceFormat format,
                                  const void* data)
    {
        glTexSubImage2D(target, level,
                        offset.x, offset.y, size.width, size.height,
                        map_color_format(format.format), format.type, data);
        THROW_IF_GL_ERROR();
    }

    void copy_texture_sub_image_2d(GLenum target, GLint level, Position2I offset,
                                   Position2I position, Size2I size)
    {
        glCopyTexSubImage2D(target, level, position.x, position.y,
                            offset.x, offset.y, size.width, size.height);
        THROW_IF_GL_ERROR();
    }

    void generate_mip_map(GLenum target)
    {
        glGenerateMipmap(target);
        THROW_IF_GL_ERROR();
    }

    GLfloat get_texture_float_parameter(GLenum target, GLenum pname)
    {
        GLfloat result;
        glGetTexParameterfv(target, pname, &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_texture_float_parameter(GLenum target, GLenum pname,
                                     GLfloat param)
    {
        glTexParameterf(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    GLint get_texture_int_parameter(GLenum target, GLenum pname)
    {
        GLint result;
        glGetTexParameteriv(target, pname, &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_texture_int_parameter(GLenum target, GLenum pname,
                                   GLint param)
    {
        glTexParameteri(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    GLint get_mag_filter(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_MAG_FILTER);
    }

    void set_mag_filter(GLenum target, GLint param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_MAG_FILTER, param);
    }

    GLint get_min_filter(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_MIN_FILTER);
    }

    void set_min_filter(GLenum target, GLint param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_MIN_FILTER, param);
    }

    GLint get_wrap_s(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_WRAP_S);
    }

    void set_wrap_s(GLenum target, GLint param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_WRAP_S, param);
    }

    GLint get_wrap_t(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_WRAP_T);
    }

    void set_wrap_t(GLenum target, GLint param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_WRAP_T, param);
    }
}
