//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlTypes.hpp"
#include <string>

#include "Tungsten/Viewport.hpp"

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

    void set_viewport(const Viewport& viewport);

    bool get_boolean_value(unsigned parameter_name);

    float get_float_value(unsigned parameter_name);

    int32_t get_int32_value(unsigned parameter_name);

    int64_t get_int64_value(unsigned parameter_name);

    std::string get_string_value(unsigned parameter_name);

    bool is_enabled(unsigned capability);

    void set_enabled(unsigned capability, bool enabled);

    void enable(unsigned capability);

    void disable(unsigned capability);

    class BlendRestorer
    {
    public:
        BlendRestorer();

        ~BlendRestorer();
    private:
        bool was_enabled_;
        int32_t src_rgb_;
        int32_t dst_rgb_;
        int32_t src_alpha_;
        int32_t dst_alpha_;
    };
}
