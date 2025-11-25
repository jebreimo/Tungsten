//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>

namespace Tungsten
{
    void draw_array(GLenum mode, int32_t offset, int32_t count);

    void draw_elements(GLenum mode, GLenum type,
                       int32_t offset, int32_t count);

    void draw_elements_16(GLenum mode, int32_t offset, int32_t count);

    inline void draw_line_array(int32_t offset, int32_t count)
    {
        draw_array(GL_LINES, offset, count);
    }

    inline void draw_line_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(GL_LINES, offset, count);
    }

    inline void draw_polygon_array(int32_t offset, int32_t count)
    {
        draw_array(GL_LINE_LOOP, offset, count);
    }

    inline void draw_polygon_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(GL_LINE_LOOP, offset, count);
    }

    inline void draw_line_strip_array(int32_t offset, int32_t count)
    {
        draw_array(GL_LINE_STRIP, offset, count);
    }

    inline void draw_line_strip_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(GL_LINE_STRIP, offset, count);
    }

    inline void draw_point_array(int32_t offset, int32_t count)
    {
        draw_array(GL_POINTS, offset, count);
    }

    inline void draw_point_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(GL_POINTS, offset, count);
    }

    inline void draw_triangle_array(int32_t offset, int32_t count)
    {
        draw_array(GL_TRIANGLES, offset, count);
    }

    inline void draw_triangle_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(GL_TRIANGLES, offset, count);
    }

    inline void draw_triangle_strip_array(int32_t offset, int32_t count)
    {
        draw_array(GL_TRIANGLE_STRIP, offset, count);
    }

    inline void draw_triangle_strip_elements_16(int32_t offset, int32_t count)
    {
        draw_elements_16(GL_TRIANGLE_STRIP, offset, count);
    }
}
