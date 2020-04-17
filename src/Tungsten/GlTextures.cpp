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

    void activateTexture(GLenum texture)
    {
        glActiveTexture(texture);
        THROW_IF_GL_ERROR();
    }

    void bindTexture(GLenum target, GLuint texture)
    {
        glBindTexture(target, texture);
        THROW_IF_GL_ERROR();
    }

    void generateMipMap(GLenum target)
    {
        glGenerateMipmap(target);
        THROW_IF_GL_ERROR();
    }

    TextureHandle generateTexture()
    {
        GLuint id;
        glGenTextures(1, &id);
        THROW_IF_GL_ERROR();
        return TextureHandle(id);
    }

    std::vector<TextureHandle> generateTextures(GLsizei count)
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

    void setTextureImage2D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLsizei height,
                           GLenum format, GLenum type, GLvoid* data)
    {
        glTexImage2D(target, level, internalFormat, width, height, 0,
                     format, type, data);
        THROW_IF_GL_ERROR();
    }

    void setTextureMagFilter(GLenum target, GLint param)
    {
        setTextureParameter(target, GL_TEXTURE_MAG_FILTER, param);
    }

    void setTextureMinFilter(GLenum target, GLint param)
    {
        setTextureParameter(target, GL_TEXTURE_MIN_FILTER, param);
    }

    void setTextureParameter(GLenum target, GLenum pname, GLfloat param)
    {
        glTexParameterf(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    void setTextureParameter(GLenum target, GLenum pname, GLint param)
    {
        glTexParameteri(target, pname, param);
        THROW_IF_GL_ERROR();
    }

    void setTextureSubImage2D(GLenum target, GLint level, GLint xOffset,
                              GLint yOffset, GLsizei width, GLsizei height,
                              GLenum format, GLenum type, GLvoid* data)
    {
        glTexSubImage2D(target, level, xOffset, yOffset, width, height,
                        format, type, data);
        THROW_IF_GL_ERROR();
    }
}
