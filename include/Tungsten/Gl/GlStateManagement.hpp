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
    [[nodiscard]] bool is_blend_enabled();

    void set_blend_enabled(bool enabled);

    void set_blend_function(BlendFunction src, BlendFunction dst);

    [[nodiscard]] bool is_depth_test_enabled();

    void set_depth_test_enabled(bool enabled);

    [[nodiscard]] bool is_face_culling_enabled();

    void set_face_culling_enabled(bool enabled);

    void set_face_culling_mode(FaceCullingMode mode);

    [[nodiscard]] bool is_multisampling_enabled();

    void set_multisampling_enabled(bool enabled);

    void set_viewport(int x, int y, int width, int height);

    void set_viewport(const Viewport& viewport);

    [[nodiscard]] bool get_boolean_value(unsigned parameter_name);

    [[nodiscard]] float get_float_value(unsigned parameter_name);

    [[nodiscard]] int32_t get_int32_value(unsigned parameter_name);

    [[nodiscard]] int64_t get_int64_value(unsigned parameter_name);

    [[nodiscard]] std::string get_string_value(unsigned parameter_name);

    [[nodiscard]] bool is_enabled(unsigned capability);

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
