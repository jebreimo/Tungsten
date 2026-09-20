//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "../Gpu/GpuTypes.hpp"

#include "../Viewport.hpp"

namespace Tungsten
{
    [[nodiscard]] bool is_blend_enabled();

    void set_blend_enabled(bool enabled);

    void set_blend_function(BlendFunction src, BlendFunction dst);

    /**
     * Sets the colour and alpha blend factors independently. The
     * two-argument set_blend_function is the common case where they agree.
     */
    void set_blend_function_separate(BlendFunction src_color,
                                     BlendFunction dst_color,
                                     BlendFunction src_alpha,
                                     BlendFunction dst_alpha);

    void set_blend_equation(BlendEquation color, BlendEquation alpha);

    [[nodiscard]] bool is_depth_test_enabled();

    void set_depth_test_enabled(bool enabled);

    void set_depth_function(CompareFunction func);

    [[nodiscard]] bool is_depth_mask_enabled();

    /**
     * Enables or disables writing to the depth buffer. Distinct from the depth
     * *test*, which keeps reading it: a blended pass normally leaves the test
     * on and the mask off, so transparent surfaces are still occluded by
     * opaque geometry but do not occlude each other — their order is decided
     * by the back-to-front sort instead.
     */
    void set_depth_mask_enabled(bool enabled);

    [[nodiscard]] bool is_face_culling_enabled();

    void set_face_culling_enabled(bool enabled);

    void set_face_culling_mode(FaceCullingMode mode);

    void set_front_face(FrontFace front_face);

    void set_color_write_mask(ColorWriteMask mask);

    [[nodiscard]] bool is_multisampling_enabled();

    void set_multisampling_enabled(bool enabled);

    [[nodiscard]] bool is_scissor_enabled();

    /**
     * Enables or disables the scissor test. Unlike the pipeline state beside
     * it this belongs to a render pass, not to a draw: Metal and Vulkan both
     * treat the scissor rectangle as encoder state.
     */
    void set_scissor_enabled(bool enabled);

    void set_scissor(int x, int y, int width, int height);

    void set_viewport(int x, int y, int width, int height);

    void set_viewport(const Viewport& viewport);

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
