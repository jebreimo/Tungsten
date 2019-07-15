//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlTextureDeleter
    {
        void operator()(GLuint id) const;
    };

    using TextureHandle = GlHandle<GlTextureDeleter>;

    void activateTexture(GLenum texture);

    void bindTexture(GLenum target, GLuint texture);

    void generateMipMap(GLenum target);

    TextureHandle generateTexture();

    std::vector<TextureHandle> generateTextures(GLsizei count);

    void setTextureImage2D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLsizei height,
                           GLenum format, GLenum type, GLvoid* data);

    void setTextureMagFilter(GLenum target, GLint param);

    void setTextureMinFilter(GLenum target, GLint param);

    void setTextureParameter(GLenum target, GLenum pname, GLfloat param);

    void setTextureParameter(GLenum target, GLenum pname, GLint param);

    void setTextureSubImage2D(GLenum target, GLint level, GLint xOffset,
                              GLint yOffset, GLsizei width, GLsizei height,
                              GLenum format, GLenum type, GLvoid* data);
}
