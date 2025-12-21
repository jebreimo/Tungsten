//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <SDL3/SDL.h>

namespace Tungsten
{
    struct GlVersion
    {
        explicit constexpr operator bool() const
        {
            return major_version != 0;
        }

        SDL_GLProfile profile = SDL_GLProfile(0);
        int major_version = 0;
        int minor_version = 0;
        [[nodiscard]] bool is_es() const;
        [[nodiscard]] bool is_core() const;
    };

    GlVersion get_sdl_gl_version();

    void set_sdl_gl_version(const GlVersion& version);

    enum class GlVersionCode
    {
        ES_2,
        ES_3,
        CORE_3_1,
        CORE_4_1,
        WEBGL_1 = ES_2,
        WEBGL_2 = ES_3
    };

    GlVersionCode get_sdl_gl_version_code();

    GlVersion make_gl_version(GlVersionCode version_code);

    struct GlMultiSampling
    {
        explicit constexpr operator bool() const
        {
            return buffers != 0 && samples != 0;
        }

        int buffers = 0;
        int samples = 0;
    };

    GlMultiSampling get_sdl_gl_multi_sampling();

    void set_sdl_gl_multi_sampling(const GlMultiSampling& multi_sampling);

    struct GlParamaters
    {
        GlVersion version;
        GlMultiSampling multi_sampling;
    };

    void set_sdl_gl_parameters(const GlParamaters& params);
}
