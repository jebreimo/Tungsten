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
    void draw_elements(GLenum mode, GLsizei count,
                       GLenum type, GLsizei offset = 0);

    inline void draw_lines16(GLsizei count, GLsizei offset = 0)
    {
        draw_elements(GL_LINES, count, GL_UNSIGNED_SHORT, offset);
    }

    inline void draw_line_loop16(GLsizei count, GLsizei offset = 0)
    {
        draw_elements(GL_LINE_LOOP, count, GL_UNSIGNED_SHORT, offset);
    }

    inline void draw_line_strip16(GLsizei count, GLsizei offset = 0)
    {
        draw_elements(GL_LINE_STRIP, count, GL_UNSIGNED_SHORT, offset);
    }

    inline void draw_points16(GLsizei count, GLsizei offset = 0)
    {
        draw_elements(GL_POINTS, count, GL_UNSIGNED_SHORT, offset);
    }

    inline void draw_triangles16(GLsizei count, GLsizei offset = 0)
    {
        draw_elements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, offset);
    }

    inline void draw_triangle_strip16(GLsizei count, GLsizei offset = 0)
    {
        draw_elements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_SHORT, offset);
    }

    void define_vertex_attribute_pointer(GLuint attribute_location, GLint size,
                                         GLenum type, bool normalized,
                                         GLsizei stride, size_t offset = 0);

    void enable_vertex_attribute(GLuint location);

    void disable_vertex_attribute(GLuint location);
}
