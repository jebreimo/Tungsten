//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertexArray.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlVertexArrayDeleter::operator()(GLuint id) const
    {
        glDeleteVertexArrays(1, &id);
        THROW_IF_GL_ERROR();
    }

    VertexArrayHandle generate_vertex_array()
    {
        GLuint id;
        glGenVertexArrays(1, &id);
        THROW_IF_GL_ERROR();
        return VertexArrayHandle(id);
    }

    void generate_vertex_arrays(std::span<VertexArrayHandle> vertex_arrays)
    {
        auto ids = std::vector<GLuint>(vertex_arrays.size());
        glGenVertexArrays(static_cast<GLsizei>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            vertex_arrays[i] = VertexArrayHandle(ids[i]);
    }

    void bind_vertex_array(GLuint vertex_array)
    {
        glBindVertexArray(vertex_array);
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

    void define_vertex_attribute_pointer(GLuint location,
                                         GLint size,
                                         GLenum type,
                                         GLsizei stride,
                                         size_t offset,
                                         bool normalized)
    {
        glVertexAttribPointer(location, size, type,
                              GLboolean(normalized ? 1 : 0),
                              stride,
                              reinterpret_cast<void*>(offset));
        THROW_IF_GL_ERROR();
    }

    void define_vertex_attribute_float_pointer(GLuint location,
                                               GLint size,
                                               GLsizei stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_FLOAT,
                                        stride, offset);
    }

    void define_vertex_attribute_int16_pointer(GLuint location,
                                               GLint size,
                                               GLsizei stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_SHORT,
                                        stride, offset);
    }

    void define_vertex_attribute_int32_pointer(GLuint location,
                                               GLint size,
                                               GLsizei stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_INT,
                                        stride, offset);
    }

    GLsizei get_size_of_type(GLenum type)
    {
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);
        case GL_SHORT:
            return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
            return sizeof(GLint);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        default:
            TUNGSTEN_THROW("Unknown type in get_size_of_type: " + std::to_string(type));
        }
    }

    bool is_float_type(GLenum type)
    {
        return type == GL_FLOAT || type == GL_DOUBLE;
    }
}
