//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL3/SDL.h>

namespace Tungsten
{
    class SdlGlContext
    {
    public:
        static SdlGlContext create(SDL_Window* window);

        SdlGlContext();

        SdlGlContext(SdlGlContext& other) = delete;

        SdlGlContext(SdlGlContext&& other) noexcept;

        ~SdlGlContext();

        SdlGlContext& operator=(SdlGlContext& other) = delete;

        SdlGlContext& operator=(SdlGlContext&& other) noexcept;

        operator SDL_GLContext() const;

    private:
        SdlGlContext(SDL_Window* window);

        SDL_GLContext context_;
    };
}
