//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL2/SDL.h>

namespace Tungsten
{
    class GlContext
    {
    public:
        static GlContext create(SDL_Window* window);

        GlContext();

        GlContext(GlContext& other) = delete;

        GlContext(GlContext&& other) noexcept;

        ~GlContext();

        GlContext& operator=(GlContext& other) = delete;

        GlContext& operator=(GlContext&& other) noexcept;

        operator SDL_GLContext() const;

    private:
        GlContext(SDL_Window* window);

        SDL_GLContext context_;
    };
}
