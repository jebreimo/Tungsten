//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL2/SDL.h>

namespace Tungsten
{
    struct GlVersion
    {
        explicit constexpr operator bool() const
        {
            return major_version != 0;
        }

        SDL_GLprofile profile = SDL_GLprofile(0);
        int major_version = 0;
        int minor_version = 0;
    };

    enum class GlVersionCode
    {
        ES_2,
        CORE_3_1,
        CORE_4_1
    };

    GlVersion get_default_gl_version(GlVersionCode version_code);

    GlVersion get_sdl_gl_version();

    void set_sdl_gl_version(GlVersion version);
}
