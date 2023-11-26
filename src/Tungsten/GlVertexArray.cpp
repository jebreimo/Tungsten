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

    std::vector<VertexArrayHandle> generate_vertex_arrays(GLsizei count)
    {
        if (count == 0)
            return {};

        auto ids = std::vector<GLuint>(size_t(count));
        glGenVertexArrays(count, ids.data());
        THROW_IF_GL_ERROR();
        auto result = std::vector<VertexArrayHandle>();
        for (auto id : ids)
            result.emplace_back(id);
        return result;
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
                                         bool normalized,
                                         GLsizei stride,
                                         size_t offset)
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
                                        false, stride, offset);
    }

    void define_vertex_attribute_int32_pointer(GLuint location,
                                               GLint size,
                                               GLsizei stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_INT,
                                        false, stride, offset);
    }
}
