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
            return majorVersion != 0;
        }

        SDL_GLprofile profile = SDL_GLprofile(0);
        int majorVersion = 0;
        int minorVersion = 0;
    };

    enum class GlVersionCode
    {
        ES_2,
        CORE_3_1,
        CORE_4_1
    };

    GlVersion getDefaultGlVersion(GlVersionCode versionCode);

    GlVersion getSdlGlVersion();

    void setSdlGlVersion(GlVersion version);
}
