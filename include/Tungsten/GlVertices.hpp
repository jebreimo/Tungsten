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
    void draw_array(GLenum mode, GLsizei offset, GLsizei count);

    void draw_elements(GLenum mode, GLenum type,
                       GLsizei offset, GLsizei count);

    void draw_elements_16(GLenum mode, GLsizei offset, GLsizei count);

    inline void draw_line_array(GLsizei offset, GLsizei count)
    {
        draw_array(GL_LINES, offset, count);
    }

    inline void draw_line_elements_16(GLsizei offset, GLsizei count)
    {
        draw_elements_16(GL_LINES, offset, count);
    }

    inline void draw_polygon_array(GLsizei offset, GLsizei count)
    {
        draw_array(GL_LINE_LOOP, offset, count);
    }

    inline void draw_polygon_elements_16(GLsizei offset, GLsizei count)
    {
        draw_elements_16(GL_LINE_LOOP, offset, count);
    }

    inline void draw_line_strip_array(GLsizei offset, GLsizei count)
    {
        draw_array(GL_LINE_STRIP, offset, count);
    }

    inline void draw_line_strip_elements_16(GLsizei offset, GLsizei count)
    {
        draw_elements_16(GL_LINE_STRIP, offset, count);
    }

    inline void draw_point_array(GLsizei offset, GLsizei count)
    {
        draw_array(GL_POINTS, offset, count);
    }

    inline void draw_point_elements_16(GLsizei offset, GLsizei count)
    {
        draw_elements_16(GL_POINTS, offset, count);
    }

    inline void draw_triangle_array(GLsizei offset, GLsizei count)
    {
        draw_array(GL_TRIANGLES, offset, count);
    }

    inline void draw_triangle_elements_16(GLsizei offset, GLsizei count)
    {
        draw_elements_16(GL_TRIANGLES, offset, count);
    }

    inline void draw_triangle_strip_array(GLsizei offset, GLsizei count)
    {
        draw_array(GL_TRIANGLE_STRIP, offset, count);
    }

    inline void draw_triangle_strip_elements_16(GLsizei offset, GLsizei count)
    {
        draw_elements_16(GL_TRIANGLE_STRIP, offset, count);
    }

    void define_vertex_attribute_pointer(GLuint attribute_location,
                                         GLint size,
                                         GLenum type,
                                         bool normalized,
                                         GLsizei stride,
                                         size_t offset = 0);

    void define_vertex_attribute_float_pointer(GLuint attribute_location,
                                               GLint size,
                                               GLsizei stride,
                                               size_t offset = 0);

    void enable_vertex_attribute(GLuint location);

    void disable_vertex_attribute(GLuint location);
}
