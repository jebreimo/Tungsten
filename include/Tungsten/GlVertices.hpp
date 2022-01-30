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

    void define_vertex_attribute_pointer(GLuint attribute_location, GLint size,
                                         GLenum type, bool normalized,
                                         GLsizei stride, size_t offset = 0);

    void enable_vertex_attribute(GLuint location);

    void disable_vertex_attribute(GLuint location);
}
