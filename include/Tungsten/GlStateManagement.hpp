//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlTypes.hpp"

namespace Tungsten
{
    bool is_blend_enabled();

    void set_blend_enabled(bool enabled);

    void set_blend_function(BlendFunction src, BlendFunction dst);

    bool is_depth_test_enabled();

    void set_depth_test_enabled(bool enabled);

    bool is_face_culling_enabled();

    void set_face_culling_enabled(bool enabled);

    void set_face_culling_mode(FaceCullingMode mode);

    void set_viewport(int x, int y, int width, int height);
}
