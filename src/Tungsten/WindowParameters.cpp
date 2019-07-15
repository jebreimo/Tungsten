//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/WindowParameters.hpp"

namespace Tungsten
{
    WindowRectangle::WindowRectangle(int width, int height)
        : x(SDL_WINDOWPOS_CENTERED),
          y(SDL_WINDOWPOS_CENTERED),
          width(width),
          height(height)
    {}

    WindowRectangle::WindowRectangle(int x, int y, int width, int height)
        : x(SDL_WINDOWPOS_CENTERED),
          y(SDL_WINDOWPOS_CENTERED),
          width(width),
          height(height)
    {}

    WindowParameters::WindowParameters(const std::string& title,
                                       const WindowRectangle& windowRectangle,
                                       uint32_t flags)
        : title(title),
          windowRectangle(windowRectangle),
          flags(flags)
    {}
}
