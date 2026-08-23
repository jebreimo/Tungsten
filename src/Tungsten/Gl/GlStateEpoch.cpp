//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlStateEpoch.hpp"

namespace Tungsten
{
    namespace
    {
        uint64_t g_gl_state_epoch = 0;
    }

    void notify_gl_state_changed()
    {
        ++g_gl_state_epoch;
    }

    uint64_t gl_state_epoch()
    {
        return g_gl_state_epoch;
    }
} // Tungsten
