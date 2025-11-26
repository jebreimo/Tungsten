//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlTexture.hpp"

#include <vector>

#include "GlTypeConversion.h"
#include "Tungsten/Environment.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        GLenum to_ogl_internal_format(TextureFormat format)
        {
            return to_ogl_texture_format(format);
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

    void bind_texture(TextureTarget target, uint32_t texture)
    {
        glBindTexture(to_ogl_texture_target(target), texture);
        THROW_IF_GL_ERROR();
    }

    int32_t active_texture_unit()
    {
        int32_t result;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &result);
        THROW_IF_GL_ERROR();
        return result - GL_TEXTURE0;
    }

    uint32_t bound_texture(TextureTarget target)
    {
        int32_t result;
        glGetIntegerv(map_texture_binding(to_ogl_texture_target(target)), &result);
        THROW_IF_GL_ERROR();
        return static_cast<uint32_t>(result);
    }

    void set_texture_image_2d(TextureTarget2D target, int32_t level,
                              Size2I size,
                              TextureSourceFormat format,
                              const void* data)
    {
        glTexImage2D(to_ogl_texture_target_2d(target), level,
                     to_ogl_internal_format(format.format),
                     size.x(), size.y(), 0,
                     to_ogl_texture_format(format.format), to_ogl_texture_value_type(format.type), data);
        THROW_IF_GL_ERROR();
    }

    void set_texture_storage_2d(TextureTarget2D target, int32_t levels, TextureFormat format, Size2I size)
    {
        glTexStorage2D(to_ogl_texture_target_2d(target), levels,
                       to_ogl_internal_format(format),
                       size.x(), size.y());
        THROW_IF_GL_ERROR();
    }

    void set_texture_sub_image_2d(TextureTarget2D target, int32_t level,
                                  Position2I offset,
                                  Size2I size,
                                  TextureSourceFormat format,
                                  const void* data)
    {
        glTexSubImage2D(to_ogl_texture_target_2d(target), level,
                        offset.x(), offset.y(), size.x(), size.y(),
                        to_ogl_texture_format(format.format), to_ogl_texture_value_type(format.type), data);
        THROW_IF_GL_ERROR();
    }

    void copy_texture_sub_image_2d(TextureTarget2D target, int32_t level, Position2I offset,
                                   Position2I position, Size2I size)
    {
        glCopyTexSubImage2D(to_ogl_texture_target_2d(target), level, position.x(), position.y(),
                            offset.x(), offset.y(), size.x(), size.y());
        THROW_IF_GL_ERROR();
    }

    void generate_mip_map(TextureTarget target)
    {
        glGenerateMipmap(to_ogl_texture_target(target));
        THROW_IF_GL_ERROR();
    }

    float get_texture_float_parameter(TextureTarget target, TextureParameter pname)
    {
        float result;
        glGetTexParameterfv(to_ogl_texture_target(target), to_ogl_texture_parameter(pname), &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_texture_float_parameter(TextureTarget target, TextureParameter pname,
                                     float param)
    {
        glTexParameterf(to_ogl_texture_target(target), to_ogl_texture_parameter(pname), param);
        THROW_IF_GL_ERROR();
    }

    int32_t get_texture_int_parameter(TextureTarget target, TextureParameter pname)
    {
        int32_t result;
        glGetTexParameteriv(to_ogl_texture_target(target), to_ogl_texture_parameter(pname), &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_texture_int_parameter(TextureTarget target, TextureParameter pname,
                                   int32_t param)
    {
        glTexParameteri(to_ogl_texture_target(target), to_ogl_texture_parameter(pname), param);
        THROW_IF_GL_ERROR();
    }

    int32_t get_mag_filter(TextureTarget target)
    {
        return get_texture_int_parameter(target, TextureParameter::MAG_FILTER);
    }

    void set_mag_filter(TextureTarget target, int32_t param)
    {
        set_texture_int_parameter(target, TextureParameter::MAG_FILTER, param);
    }

    int32_t get_min_filter(TextureTarget target)
    {
        return get_texture_int_parameter(target, TextureParameter::MIN_FILTER);
    }

    void set_min_filter(TextureTarget target, int32_t param)
    {
        set_texture_int_parameter(target, TextureParameter::MIN_FILTER, param);
    }

    int32_t get_wrap_s(TextureTarget target)
    {
        return get_texture_int_parameter(target, TextureParameter::WRAP_S);
    }

    void set_wrap_s(TextureTarget target, int32_t param)
    {
        set_texture_int_parameter(target, TextureParameter::WRAP_S, param);
    }

    int32_t get_wrap_t(TextureTarget target)
    {
        return get_texture_int_parameter(target, TextureParameter::WRAP_T);
    }

    void set_wrap_t(TextureTarget target, int32_t param)
    {
        set_texture_int_parameter(target, TextureParameter::WRAP_T, param);
    }
}
