//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-12-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

namespace Tungsten
{
    constexpr bool is_emscripten()
    {
    #ifdef EMSCRIPTEN
        return true;
    #else
        return false;
    #endif
    }
}
