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
    struct WindowRectangle
    {
        WindowRectangle(int width = 800, int height = 600);

        WindowRectangle(int x, int y, int width, int height);

        int x;
        int y;
        int width;
        int height;
    };

    struct WindowParameters
    {
        WindowParameters(
                const std::string& title = "SdlApplication",
                const WindowRectangle& windowRectangle = WindowRectangle(),
                uint32_t flags = SDL_WINDOW_OPENGL);

        std::string title;
        WindowRectangle windowRectangle;
        uint32_t flags;
    };
}
