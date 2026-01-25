//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-25.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Viewport.hpp"

namespace Tungsten
{
    std::ostream &operator<<(std::ostream &os, const Viewport &viewport)
    {
        os << "{origin: " << viewport.origin << ", size: " << viewport.size << "}";
        return os;
    }
}
