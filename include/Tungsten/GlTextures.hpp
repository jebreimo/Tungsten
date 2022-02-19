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

    void activate_texture(GLenum texture);

    void bind_texture(GLenum target, GLuint texture);

    void generate_mip_map(GLenum target);

    TextureHandle generate_texture();

    std::vector<TextureHandle> generate_textures(GLsizei count);

    void set_texture_image_2d(GLenum target, GLint level,
                              GLint internal_format, GLsizei width,
                              GLsizei height, GLenum format,
                              GLenum type, const void* data);

    void set_texture_mag_filter(GLenum target, GLint param);

    void set_texture_min_filter(GLenum target, GLint param);

    void set_texture_parameter(GLenum target, GLenum pname, GLfloat param);

    void set_texture_parameter(GLenum target, GLenum pname, GLint param);

    void set_texture_sub_image_2d(GLenum target, GLint level,
                                  GLint x_offset, GLint y_offset,
                                  GLsizei width, GLsizei height,
                                  GLenum format, GLenum type,
                                  const void* data);
}
