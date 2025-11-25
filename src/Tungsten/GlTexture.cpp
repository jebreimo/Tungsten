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

        GLenum get_ogl_color_format(TextureFormat format)
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

        GLenum get_ogl_internal_format(TextureFormat format)
        {
            return get_ogl_color_format(format);
        }

        GLenum get_ogl_type(TextureType type)
        {
            switch (type)
            {
            case TextureType::UINT8:
                    return GL_UNSIGNED_BYTE;
            case TextureType::FLOAT:
                return GL_FLOAT;
            default:
                TUNGSTEN_THROW("Unsupported texture type: " + std::to_string(static_cast<int>(type)));
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

    void TextureDeleter::operator()(uint32_t id) const
    {
        glDeleteTextures(1, &id);
        THROW_IF_GL_ERROR();
    }

    TextureHandle generate_texture()
    {
        uint32_t id;
        glGenTextures(1, &id);
        THROW_IF_GL_ERROR();
        return TextureHandle(id);
    }

    void generate_textures(std::span<TextureHandle> textures)
    {
        auto ids = std::vector<uint32_t>(textures.size());
        glGenTextures(static_cast<int32_t>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            textures[i] = TextureHandle(ids[i]);
    }

    void activate_texture_unit(int32_t unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        THROW_IF_GL_ERROR();
    }

    void bind_texture(GLenum target, uint32_t texture)
    {
        glBindTexture(target, texture);
        THROW_IF_GL_ERROR();
    }

    int32_t active_texture_unit()
    {
        int32_t result;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &result);
        THROW_IF_GL_ERROR();
        return result - GL_TEXTURE0;
    }

    uint32_t bound_texture(GLenum target)
    {
        int32_t result;
        glGetIntegerv(map_texture_binding(target), &result);
        THROW_IF_GL_ERROR();
        return static_cast<uint32_t>(result);
    }

    void set_texture_image_2d(GLenum target, int32_t level,
                              int32_t internal_format,
                              Size2I size,
                              TextureSourceFormat format,
                              const void* data)
    {
        glTexImage2D(target, level,
                     static_cast<int32_t>(map_color_format(internal_format)),
                     size.x(), size.y(), 0,
                     get_ogl_color_format(format.format), get_ogl_type(format.type), data);
        THROW_IF_GL_ERROR();
    }

    void set_texture_storage_2d(GLenum target, int32_t levels, int32_t internal_format, Size2I size)
    {
        glTexStorage2D(target, levels,
                       static_cast<int32_t>(map_color_format(internal_format)),
                       size.x(), size.y());
        THROW_IF_GL_ERROR();
    }

    void set_texture_sub_image_2d(GLenum target, int32_t level,
                                  Position2I offset,
                                  Size2I size,
                                  TextureSourceFormat format,
                                  const void* data)
    {
        glTexSubImage2D(target, level,
                        offset.x(), offset.y(), size.x(), size.y(),
                        get_ogl_color_format(format.format), get_ogl_type(format.type), data);
        THROW_IF_GL_ERROR();
    }

    void copy_texture_sub_image_2d(GLenum target, int32_t level, Position2I offset,
                                   Position2I position, Size2I size)
    {
        glCopyTexSubImage2D(target, level, position.x(), position.y(),
                            offset.x(), offset.y(), size.x(), size.y());
        THROW_IF_GL_ERROR();
    }

    void generate_mip_map(GLenum target)
    {
        glGenerateMipmap(target);
        THROW_IF_GL_ERROR();
    }

    float get_texture_float_parameter(GLenum target, GLenum pname)
    {
        float result;
        glGetTexParameterfv(target, pname, &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_texture_float_parameter(GLenum target, GLenum pname,
                                     float param)
    {
        glTexParameterf(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    int32_t get_texture_int_parameter(GLenum target, GLenum pname)
    {
        int32_t result;
        glGetTexParameteriv(target, pname, &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_texture_int_parameter(GLenum target, GLenum pname,
                                   int32_t param)
    {
        glTexParameteri(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    int32_t get_mag_filter(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_MAG_FILTER);
    }

    void set_mag_filter(GLenum target, int32_t param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_MAG_FILTER, param);
    }

    int32_t get_min_filter(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_MIN_FILTER);
    }

    void set_min_filter(GLenum target, int32_t param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_MIN_FILTER, param);
    }

    int32_t get_wrap_s(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_WRAP_S);
    }

    void set_wrap_s(GLenum target, int32_t param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_WRAP_S, param);
    }

    int32_t get_wrap_t(GLenum target)
    {
        return get_texture_int_parameter(target, GL_TEXTURE_WRAP_T);
    }

    void set_wrap_t(GLenum target, int32_t param)
    {
        set_texture_int_parameter(target, GL_TEXTURE_WRAP_T, param);
    }
}
