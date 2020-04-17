//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/WindowParameters.hpp"

#include <utility>

namespace Tungsten
{
    WindowParameters::WindowParameters() = default;

    WindowParameters::WindowParameters(std::string title,
                                       WindowPos windowPos,
                                       WindowSize windowSize,
                                       uint32_t sdlFlags,
                                       FullScreenMode fullScreenMode)
        : title(std::move(title)),
          windowPos(windowPos),
          windowSize(windowSize),
          sdlFlags(sdlFlags),
          fullScreenMode(fullScreenMode)
    {}
}
