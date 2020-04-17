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

        int width = 640;
        int height = 480;
    };

    //struct WindowRectangle
    //{
    //    explicit WindowRectangle(int width = 800, int height = 600);
    //
    //    explicit WindowRectangle(int x, int y, int width, int height);
    //
    //    int x;
    //    int y;
    //    int width;
    //    int height;
    //};

    struct FullScreenMode
    {
        FullScreenMode() = default;

        FullScreenMode(int display, int mode)
            : displayIndex(display), modeIndex(mode)
        {}

        explicit operator bool() const
        {
            return displayIndex >= 0 && modeIndex >= 0;
        }

        int displayIndex = -1;
        int modeIndex = -1;
    };

    struct WindowParameters
    {
        WindowParameters();

        explicit WindowParameters(
                std::string title,
                WindowPos windowPos = {},
                WindowSize windowSize = {},
                uint32_t sdlFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
                FullScreenMode fullScreenMode = {});

        std::string title;
        WindowPos windowPos;
        WindowSize windowSize;
        uint32_t sdlFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        FullScreenMode fullScreenMode;
    };
}
