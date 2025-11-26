//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>

#include "Types.hpp"

namespace Tungsten
{
    void draw_array(TopologyType topology, int32_t offset, int32_t count);

    void draw_elements(TopologyType topology, GLenum type,
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
