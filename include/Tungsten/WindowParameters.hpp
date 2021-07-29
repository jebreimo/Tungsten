//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

namespace Tungsten
{
    struct WindowPos
    {
        WindowPos() = default;

        WindowPos(int x, int y) : x(x), y(y) {}

        int x = SDL_WINDOWPOS_UNDEFINED;
        int y = SDL_WINDOWPOS_UNDEFINED;
    };

    struct WindowSize
    {
        WindowSize() = default;

        WindowSize(int w, int h) : width(w), height(h) {}

        explicit operator bool() const
        {
            return width > 0 && height > 0;
        }

        int width = -1;
        int height = -1;
    };

    struct FullScreenMode
    {
        FullScreenMode() = default;

        FullScreenMode(int display, int mode)
            : display(display), mode(mode)
        {}

        explicit operator bool() const
        {
            return display >= 0 && mode >= 0;
        }

        int display = -1;
        int mode = -1;
    };

    struct WindowParameters
    {
        WindowPos windowPos;
        WindowSize windowSize;
        WindowSize defaultWindowSize = {640, 480};
        uint32_t sdlFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        FullScreenMode fullScreenMode;
    };
}
