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
        void operator()(uint32_t id) const;
    };

    using TextureHandle = GlHandle<TextureDeleter>;

    TextureHandle generate_texture();

    void generate_textures(std::span<TextureHandle> textures);

    void activate_texture_unit(int32_t unit);

    void bind_texture(GLenum target, uint32_t texture);

    [[nodiscard]] int32_t active_texture_unit();

    [[nodiscard]] uint32_t bound_texture(GLenum target);

    constexpr TextureSourceFormat RGB_TEXTURE = {TextureFormat::RGB, TextureType::UINT8};
    constexpr TextureSourceFormat RGBA_TEXTURE = {TextureFormat::RGBA, TextureType::UINT8};

    void set_texture_image_2d(GLenum target, int32_t level,
                              Size2I size,
                              TextureSourceFormat format,
                              const void* data = nullptr);

    void set_texture_storage_2d(GLenum target, int32_t levels,
                                TextureFormat format,
                                Size2I size);

    void set_texture_sub_image_2d(GLenum target, int32_t level,
                                  Position2I offset, Size2I size,
                                  TextureSourceFormat format,
                                  const void* data);

    void copy_texture_sub_image_2d(GLenum target, int32_t level,
                                   Position2I offset,
                                   Position2I position,
                                   Size2I size);

    void generate_mip_map(GLenum target);

    [[nodiscard]]
    float get_texture_float_parameter(GLenum target, GLenum pname);

    void set_texture_float_parameter(GLenum target, GLenum pname, float param);

    [[nodiscard]]
    int32_t get_texture_int_parameter(GLenum target, GLenum pname);

    void set_texture_int_parameter(GLenum target, GLenum pname, int32_t param);

    [[nodiscard]]
    int32_t get_mag_filter(GLenum target);

    void set_mag_filter(GLenum target, int32_t param);

    [[nodiscard]]
    int32_t get_min_filter(GLenum target);

    void set_min_filter(GLenum target, int32_t param);

    [[nodiscard]]
    int32_t get_wrap_s(GLenum target);

    void set_wrap_s(GLenum target, int32_t param);

    [[nodiscard]]
    int32_t get_wrap_t(GLenum target);

    void set_wrap_t(GLenum target, int32_t param);
}
