//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GlPipelineBinder.hpp"

#include "Tungsten/Gl/GlStateManagement.hpp"

namespace Tungsten
{
    void GlPipelineBinder::bind(const PipelineDescriptor& descriptor)
    {
        sync();

        // An empty cache never compares equal, so a fresh or invalidated
        // binder states every field even where it matches the GL default.
        const PipelineDescriptor* previous
            = current_ ? &*current_ : nullptr;

        const auto& depth = descriptor.depth;
        if (!previous || previous->depth.test != depth.test)
            set_depth_test_enabled(depth.test);
        if (!previous || previous->depth.write != depth.write)
            set_depth_mask_enabled(depth.write);
        if (!previous || previous->depth.compare != depth.compare)
            set_depth_function(depth.compare);

        const auto& blend = descriptor.blend;
        if (!previous || previous->blend.enabled != blend.enabled)
            set_blend_enabled(blend.enabled);
        if (blend.enabled)
        {
            // Only meaningful while blending is on, so a pipeline that does
            // not blend costs nothing to describe.
            const bool factors_differ
                = !previous
                  || previous->blend.src_color != blend.src_color
                  || previous->blend.dst_color != blend.dst_color
                  || previous->blend.src_alpha != blend.src_alpha
                  || previous->blend.dst_alpha != blend.dst_alpha;
            if (factors_differ)
            {
                set_blend_function_separate(blend.src_color, blend.dst_color,
                                            blend.src_alpha, blend.dst_alpha);
            }

            const bool equations_differ
                = !previous
                  || previous->blend.color_equation != blend.color_equation
                  || previous->blend.alpha_equation != blend.alpha_equation;
            if (equations_differ)
            {
                set_blend_equation(blend.color_equation,
                                   blend.alpha_equation);
            }
        }
        if (!previous || previous->blend.color_write != blend.color_write)
            set_color_write_mask(blend.color_write);

        const auto& raster = descriptor.raster;
        if (!previous || previous->raster.cull_enabled != raster.cull_enabled)
            set_face_culling_enabled(raster.cull_enabled);
        if (raster.cull_enabled
            && (!previous || previous->raster.cull_mode != raster.cull_mode))
        {
            set_face_culling_mode(raster.cull_mode);
        }
        if (!previous || previous->raster.front_face != raster.front_face)
            set_front_face(raster.front_face);

        current_ = descriptor;
    }

    void GlPipelineBinder::invalidate()
    {
        current_.reset();
        epoch_seen_ = gl_state_epoch();
    }

    void GlPipelineBinder::sync()
    {
        if (epoch_seen_ != gl_state_epoch())
            invalidate();
    }
} // Tungsten
