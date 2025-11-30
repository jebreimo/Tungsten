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

    void bind_texture(TextureTarget, uint32_t texture);

    [[nodiscard]] int32_t active_texture_unit();

    [[nodiscard]] uint32_t bound_texture(TextureTarget);

    constexpr TextureSourceFormat RGB_TEXTURE = {TextureFormat::RGB, TextureValueType::UINT8};
    constexpr TextureSourceFormat RGBA_TEXTURE = {TextureFormat::RGBA, TextureValueType::UINT8};

    void set_texture_image_2d(TextureTarget2D target, int32_t level,
                              Size2I size,
                              TextureSourceFormat format,
                              const void* data = nullptr);

    void set_texture_storage_2d(TextureTarget2D target, int32_t levels,
                                TextureFormat format,
                                Size2I size);

    void set_texture_sub_image_2d(TextureTarget2D target, int32_t level,
                                  Position2I offset, Size2I size,
                                  TextureSourceFormat format,
                                  const void* data);

    void copy_texture_sub_image_2d(TextureTarget2D target, int32_t level,
                                   Position2I offset,
                                   Position2I position,
                                   Size2I size);

    void generate_mip_map(TextureTarget target);

    [[nodiscard]]
    float get_texture_float_parameter(TextureTarget target, TextureParameter pname);

    void set_texture_float_parameter(TextureTarget target, TextureParameter pname, float param);

    [[nodiscard]]
    int32_t get_texture_int_parameter(TextureTarget target, TextureParameter pname);

    void set_texture_int_parameter(TextureTarget target, TextureParameter pname, int32_t param);

    [[nodiscard]]
    TextureMagFilter get_mag_filter(TextureTarget target);

    void set_mag_filter(TextureTarget target, TextureMagFilter filter);

    [[nodiscard]]
    TextureMinFilter get_min_filter(TextureTarget target);

    void set_min_filter(TextureTarget target, TextureMinFilter filter);

    [[nodiscard]]
    TextureWrapMode get_wrap_s(TextureTarget target);

    void set_wrap_s(TextureTarget target, TextureWrapMode mode);

    [[nodiscard]]
    TextureWrapMode get_wrap_t(TextureTarget target);

    void set_wrap_t(TextureTarget target, TextureWrapMode mode);
}
