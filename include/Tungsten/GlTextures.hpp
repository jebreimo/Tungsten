//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlTextureDeleter
    {
        void operator()(GLuint id) const;
    };

    using TextureHandle = GlHandle<GlTextureDeleter>;

    TextureHandle generate_texture();

    void generate_textures(std::span<TextureHandle> textures);

    class ActiveTexture
    {
    public:
        ActiveTexture();

        ActiveTexture(GLint unit, GLenum target, GLuint texture);

        ~ActiveTexture();

        ActiveTexture(const ActiveTexture&) = delete;

        ActiveTexture(ActiveTexture&& other) noexcept;

        ActiveTexture& operator=(const ActiveTexture&) = delete;

        ActiveTexture& operator=(ActiveTexture&& other) noexcept;

        explicit operator bool() const
        {
            return has_baton_;
        }

        void reset();

        void reset(GLint unit, GLenum target, GLuint texture);

        [[nodiscard]] GLint unit() const
        {
            return unit_;
        }

        [[nodiscard]] GLenum target() const
        {
            return target_;
        }

        [[nodiscard]] GLuint texture() const
        {
            return texture_;
        }

        void generate_mip_map() const;

        void set_image_2d(GLint level, GLint internal_format,
                          GLsizei width, GLsizei height,
                          GLenum format, GLenum type,
                          const void* data = nullptr) const;

        void set_sub_image_2d(GLint level,
                              GLint x_offset, GLint y_offset,
                              GLsizei width, GLsizei height,
                              GLenum format, GLenum type,
                              const void* data) const;

        [[nodiscard]]
        GLfloat float_parameter(GLenum pname) const;

        void set_float_parameter(GLenum pname, GLfloat param) const;

        [[nodiscard]]
        GLint int_parameter(GLenum pname) const;

        void set_int_parameter(GLenum pname, GLint param) const;

        [[nodiscard]]
        GLint mag_filter() const;

        void set_mag_filter(GLint param) const;

        [[nodiscard]]
        GLint min_filter() const;

        void set_min_filter(GLint param) const;

        [[nodiscard]]
        GLint wrap_s() const;

        void set_wrap_s(GLint param) const;

        [[nodiscard]]
        GLint wrap_t() const;

        void set_wrap_t(GLint param) const;
    private:
        bool take_baton();

        void release_baton();

        bool has_baton_ = false;
        GLint unit_ = -1;
        GLenum target_ = GL_NONE;
        GLuint texture_ = 0;

        static bool baton_;
    };
}
