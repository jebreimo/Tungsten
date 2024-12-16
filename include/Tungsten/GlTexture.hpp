//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>

#include "GlHandle.hpp"
#include "Types.hpp"

namespace Tungsten
{
    struct TextureDeleter
    {
        void operator()(GLuint id) const;
    };

    using TextureHandle = GlHandle<TextureDeleter>;

    TextureHandle generate_texture();

    void generate_textures(std::span<TextureHandle> textures);

    void activate_texture_unit(GLint unit);

    void bind_texture(GLenum target, GLuint texture);

    [[nodiscard]] GLint active_texture_unit();

    [[nodiscard]] GLuint bound_texture(GLenum target);

    constexpr TextureSourceFormat RGB_TEXTURE = {GL_RGB, GL_UNSIGNED_BYTE};
    constexpr TextureSourceFormat RGBA_TEXTURE = {GL_RGBA, GL_UNSIGNED_BYTE};
    constexpr TextureSourceFormat LUMINANCE_TEXTURE = {GL_LUMINANCE, GL_UNSIGNED_BYTE};

    void set_texture_image_2d(GLenum target, GLint level,
                              GLint internal_format,
                              Size2D size,
                              TextureSourceFormat format,
                              const void* data = nullptr);

    void set_texture_storage_2d(GLenum target, GLint levels,
                                GLint internal_format,
                                Size2D size);

    void set_texture_sub_image_2d(GLenum target, GLint level,
                                  Position2D offset, Size2D size,
                                  TextureSourceFormat format,
                                  const void* data);

    void copy_texture_sub_image_2d(GLenum target, GLint level,
                                   Position2D offset,
                                   Position2D position,
                                   Size2D size);

    void generate_mip_map(GLenum target);

    [[nodiscard]]
    GLfloat get_texture_float_parameter(GLenum target, GLenum pname);

    void set_texture_float_parameter(GLenum target, GLenum pname, GLfloat param);

    [[nodiscard]]
    GLint get_texture_int_parameter(GLenum target, GLenum pname);

    void set_texture_int_parameter(GLenum target, GLenum pname, GLint param);

    [[nodiscard]]
    GLint get_mag_filter(GLenum target);

    void set_mag_filter(GLenum target, GLint param);

    [[nodiscard]]
    GLint get_min_filter(GLenum target);

    void set_min_filter(GLenum target, GLint param);

    [[nodiscard]]
    GLint get_wrap_s(GLenum target);

    void set_wrap_s(GLenum target, GLint param);

    [[nodiscard]]
    GLint get_wrap_t(GLenum target);

    void set_wrap_t(GLenum target, GLint param);
}
