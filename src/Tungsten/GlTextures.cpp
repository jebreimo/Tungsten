//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlTextures.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlTextureDeleter::operator()(GLuint id) const
    {
        glDeleteTextures(1, &id);
        THROW_IF_GL_ERROR();
    }

    void activate_texture(GLenum texture)
    {
        glActiveTexture(texture);
        THROW_IF_GL_ERROR();
    }

    void bind_texture(GLenum target, GLuint texture)
    {
        glBindTexture(target, texture);
        THROW_IF_GL_ERROR();
    }

    void generate_mip_map(GLenum target)
    {
        glGenerateMipmap(target);
        THROW_IF_GL_ERROR();
    }

    TextureHandle generate_texture()
    {
        GLuint id;
        glGenTextures(1, &id);
        THROW_IF_GL_ERROR();
        return TextureHandle(id);
    }

    std::vector<TextureHandle> generate_textures(GLsizei count)
    {
        if (count == 0)
            return std::vector<TextureHandle>();

        auto ids = std::vector<GLuint>(size_t(count));
        glGenTextures(count, ids.data());
        THROW_IF_GL_ERROR();
        auto result = std::vector<TextureHandle>();
        for (auto id : ids)
            result.emplace_back(id);
        return result;
    }

    void set_texture_image_2d(GLenum target, GLint level,
                              GLint internal_format, GLsizei width,
                              GLsizei height, GLenum format,
                              GLenum type, const void* data)
    {
        glTexImage2D(target, level, internal_format, width, height, 0,
                     format, type, data);
        THROW_IF_GL_ERROR();
    }

    void set_texture_mag_filter(GLenum target, GLint param)
    {
        set_texture_parameter(target, GL_TEXTURE_MAG_FILTER, param);
    }

    void set_texture_min_filter(GLenum target, GLint param)
    {
        set_texture_parameter(target, GL_TEXTURE_MIN_FILTER, param);
    }

    void set_texture_parameter(GLenum target, GLenum pname, GLfloat param)
    {
        glTexParameterf(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    void set_texture_parameter(GLenum target, GLenum pname, GLint param)
    {
        glTexParameteri(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    void set_texture_sub_image_2d(GLenum target, GLint level, GLint x_offset,
                                  GLint y_offset, GLsizei width,
                                  GLsizei height, GLenum format,
                                  GLenum type, const void* data)
    {
        glTexSubImage2D(target, level, x_offset, y_offset, width, height,
                        format, type, data);
        THROW_IF_GL_ERROR();
    }
}
