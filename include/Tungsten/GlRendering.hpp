//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>
#include "Detail/GenericBitmaskOperators.hpp"

namespace Tungsten
{
    void set_clear_color(float r, float g, float b, float a);

    void set_clear_color(const Xyz::Vector4F& color);

    void set_clear_depth(float depth);

    void set_clear_stencil(int32_t stencil);

    enum class ClearBits
    {
        COLOR = 0x00004000,
        DEPTH = 0x00000100,
        STENCIL = 0x00000400,
        COLOR_DEPTH = COLOR | DEPTH,
        ALL = COLOR | DEPTH | STENCIL
    };

    TUNGSTEN_ENABLE_BITMASK_OPERATORS(ClearBits);

    void clear(ClearBits mask);

    void finish_rendering();
}
