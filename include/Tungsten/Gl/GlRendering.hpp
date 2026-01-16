//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>
#include "GlTypes.hpp"
#include "../Detail/GenericBitmaskOperators.hpp"

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

    void draw_array(TopologyType topology, int32_t offset, int32_t count);

    void draw_elements(TopologyType topology, ElementIndexType type,
                       int32_t offset, int32_t count);

    void draw_elements_16(TopologyType topology, int32_t offset, int32_t count);

    inline void draw_line_array(int32_t offset, int32_t count)
    {
        draw_array(TopologyType::LINES, offset, count);
    }

    inline void draw_line_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(TopologyType::LINES, offset, count);
    }

    inline void draw_polygon_array(int32_t offset, int32_t count)
    {
        draw_array(TopologyType::LINE_LOOP, offset, count);
    }

    inline void draw_polygon_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(TopologyType::LINE_LOOP, offset, count);
    }

    inline void draw_line_strip_array(int32_t offset, int32_t count)
    {
        draw_array(TopologyType::LINE_STRIP, offset, count);
    }

    inline void draw_line_strip_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(TopologyType::LINE_STRIP, offset, count);
    }

    inline void draw_point_array(int32_t offset, int32_t count)
    {
        draw_array(TopologyType::POINTS, offset, count);
    }

    inline void draw_point_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(TopologyType::POINTS, offset, count);
    }

    inline void draw_triangle_array(int32_t offset, int32_t count)
    {
        draw_array(TopologyType::TRIANGLES, offset, count);
    }

    inline void draw_triangle_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(TopologyType::TRIANGLES, offset, count);
    }

    inline void draw_triangle_strip_array(int32_t offset, int32_t count)
    {
        draw_array(TopologyType::TRIANGLE_STRIP, offset, count);
    }

    inline void draw_triangle_strip_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(TopologyType::TRIANGLE_STRIP, offset, count);
    }
}
