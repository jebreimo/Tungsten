//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlTextures.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Environment.hpp"

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
            case GL_RGB:
            case GL_RGB8:
                return GL_RGB;
            case GL_RGBA:
            case GL_RGBA8:
                return GL_RGBA;
            default:
                return format;
            }
        }

        void activate_texture(GLint unit)
        {
            glActiveTexture(GL_TEXTURE0 + unit);
            THROW_IF_GL_ERROR();
        }

        void bind_texture(GLenum target, GLuint texture)
        {
            glBindTexture(target, texture);
            THROW_IF_GL_ERROR();
        }
    }

    void GlTextureDeleter::operator()(GLuint id) const
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

    std::vector<TextureHandle> generate_textures(GLsizei count)
    {
        if (count == 0)
            return {};

        auto ids = std::vector<GLuint>(size_t(count));
        glGenTextures(count, ids.data());
        THROW_IF_GL_ERROR();
        auto result = std::vector<TextureHandle>();
        for (auto id : ids)
            result.emplace_back(id);
        return result;
    }

    void generate_textures(std::span<TextureHandle> textures)
    {
        auto ids = std::vector<GLuint>(size_t(textures.size()));
        glGenTextures(GLsizei(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            textures[i] = TextureHandle(ids[i]);
    }

    bool ActiveTexture::baton_ = true;

    ActiveTexture::ActiveTexture() = default;

    ActiveTexture::ActiveTexture(GLint unit, GLenum target, GLuint texture)
        : has_baton_(baton_),
          unit_(unit),
          target_(target),
          texture_(texture)
    {
        baton_ = false;
        if (!has_baton_)
            TUNGSTEN_THROW("ActiveTexture is already taken.");

        activate_texture(unit_);
        bind_texture(target_, texture_);
    }

    ActiveTexture::~ActiveTexture()
    {
        release_baton();
    }

    ActiveTexture::ActiveTexture(ActiveTexture&& other) noexcept
        : has_baton_(other.has_baton_),
          unit_(other.unit_),
          target_(other.target_),
          texture_(other.texture_)
    {
        other.has_baton_ = false;
    }

    ActiveTexture& ActiveTexture::operator=(ActiveTexture&& other) noexcept
    {
        if (has_baton_)
            baton_ = true;
        has_baton_ = other.has_baton_;
        unit_ = other.unit_;
        target_ = other.target_;
        texture_ = other.texture_;
        other.has_baton_ = false;
        return *this;
    }

    void ActiveTexture::reset()
    {
        release_baton();
        unit_ = -1;
        target_ = GL_NONE;
        texture_ = 0;
    }

    void ActiveTexture::reset(GLint unit, GLenum target, GLuint texture)
    {
        if (!take_baton())
            TUNGSTEN_THROW("ActiveTexture is already taken.");

        activate_texture(unit_);
        unit_ = unit;
        bind_texture(target_, texture_);
        target_ = target;
        texture_ = texture;
    }

    void ActiveTexture::generate_mip_map() const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        glGenerateMipmap(target_);
        THROW_IF_GL_ERROR();
    }

    void ActiveTexture::set_image_2d(GLint level, GLint internal_format,
                                     GLsizei width, GLsizei height,
                                     GLenum format, GLenum type,
                                     const void* data) const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        glTexImage2D(target_, level, GLint(map_color_format(internal_format)),
                     width, height, 0,
                     map_color_format(format), type, data);
        THROW_IF_GL_ERROR();
    }

    void ActiveTexture::set_sub_image_2d(GLint level, GLint x_offset,
                                        GLint y_offset, GLsizei width,
                                        GLsizei height, GLenum format,
                                        GLenum type, const void* data) const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        glTexSubImage2D(target_, level, x_offset, y_offset, width, height,
                        map_color_format(format), type, data);
        THROW_IF_GL_ERROR();
    }

    GLfloat ActiveTexture::float_parameter(GLenum pname) const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        GLfloat result;
        glGetTexParameterfv(target_, pname, &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void ActiveTexture::set_float_parameter(GLenum pname, GLfloat param) const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        glTexParameterf(target_, pname, param);
        THROW_IF_GL_ERROR();
    }

    GLint ActiveTexture::int_parameter(GLenum pname) const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        GLint result;
        glGetTexParameteriv(target_, pname, &result);
        THROW_IF_GL_ERROR();
        return result;
    }

    void ActiveTexture::set_int_parameter(GLenum pname, GLint param) const
    {
        if (!has_baton_)
            TUNGSTEN_THROW("This is not the currently active texture.");
        glTexParameteri(target_, pname, param);
        THROW_IF_GL_ERROR();
    }

    GLint ActiveTexture::mag_filter() const
    {
        return int_parameter(GL_TEXTURE_MAG_FILTER);
    }

    void ActiveTexture::set_mag_filter(GLint param) const
    {
        set_int_parameter(GL_TEXTURE_MAG_FILTER, param);
    }

    GLint ActiveTexture::min_filter() const
    {
        return int_parameter(GL_TEXTURE_MIN_FILTER);
    }

    void ActiveTexture::set_min_filter(GLint param) const
    {
        set_int_parameter(GL_TEXTURE_MIN_FILTER, param);
    }

    GLint ActiveTexture::wrap_s() const
    {
        return int_parameter(GL_TEXTURE_WRAP_S);
    }

    void ActiveTexture::set_wrap_s(GLint param) const
    {
        set_int_parameter(GL_TEXTURE_WRAP_S, param);
    }

    GLint ActiveTexture::wrap_t() const
    {
        return int_parameter(GL_TEXTURE_WRAP_T);
    }

    void ActiveTexture::set_wrap_t(GLint param) const
    {
        set_int_parameter(GL_TEXTURE_WRAP_T, param);
    }

    bool ActiveTexture::take_baton()
    {
        if (has_baton_)
            return true;
        std::swap(has_baton_,  baton_);
        return has_baton_;
    }

    void ActiveTexture::release_baton()
    {
        if (has_baton_)
            std::swap(has_baton_, baton_);
    }
}
