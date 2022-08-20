//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertices.hpp"
#include "Tungsten/TungstenException.hpp"


namespace Tungsten
{
    GLsizei get_type_size(GLenum type)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            return sizeof(unsigned short);
        case GL_UNSIGNED_BYTE:
            return 1;
        default:
            TUNGSTEN_THROW("Unsupported type: " + std::to_string(type));
        }
    }

    void draw_elements(GLenum mode, GLsizei count, GLenum type, GLsizei offset)
    {
        intptr_t tmp_offset = offset * get_type_size(type);
        glDrawElements(mode, count, type, reinterpret_cast<void*>(tmp_offset));
        THROW_IF_GL_ERROR();
    }

    void enable_vertex_attribute(GLuint location)
    {
        glEnableVertexAttribArray(location);
        THROW_IF_GL_ERROR();
    }

    void disable_vertex_attribute(GLuint location)
    {
        glDisableVertexAttribArray(location);
        THROW_IF_GL_ERROR();
    }

    void define_vertex_attribute_pointer(GLuint attribute_location,
                                         GLint size,
                                         GLenum type,
                                         bool normalized,
                                         GLsizei stride,
                                         size_t offset)
    {
        glVertexAttribPointer(attribute_location, size, type,
                              GLboolean(normalized ? 1 : 0),
                              stride,
                              reinterpret_cast<void*>(offset));
        THROW_IF_GL_ERROR();
    }
}
