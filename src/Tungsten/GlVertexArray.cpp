//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlVertexArray.hpp"

#include <vector>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlVertexArrayDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().deleteVertexArrays(1, &id);
        THROW_IF_GL_ERROR();
    }

    VertexArrayHandle generate_vertex_array()
    {
        uint32_t id;
        get_ogl_wrapper().genVertexArrays(1, &id);
        THROW_IF_GL_ERROR();
        return VertexArrayHandle(id);
    }

    void generate_vertex_arrays(std::span<VertexArrayHandle> vertex_arrays)
    {
        auto ids = std::vector<uint32_t>(vertex_arrays.size());
        get_ogl_wrapper().genVertexArrays(static_cast<int32_t>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            vertex_arrays[i] = VertexArrayHandle(ids[i]);
    }

    void bind_vertex_array(uint32_t vertex_array)
    {
        get_ogl_wrapper().bindVertexArray(vertex_array);
        THROW_IF_GL_ERROR();
    }

    void enable_vertex_attribute(uint32_t location)
    {
        get_ogl_wrapper().enableVertexAttribArray(location);
        THROW_IF_GL_ERROR();
    }

    void disable_vertex_attribute(uint32_t location)
    {
        get_ogl_wrapper().disableVertexAttribArray(location);
        THROW_IF_GL_ERROR();
    }

    void define_vertex_attribute_pointer(uint32_t location,
                                         int32_t size,
                                         GLenum type,
                                         int32_t stride,
                                         size_t offset,
                                         bool normalized)
    {
        get_ogl_wrapper().vertexAttribPointer(location, size, type,
                              GLboolean(normalized ? 1 : 0),
                              stride,
                              reinterpret_cast<void*>(offset));
        THROW_IF_GL_ERROR();
    }

    void define_vertex_attribute_float_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_FLOAT,
                                        stride, offset);
    }

    void define_vertex_attribute_int16_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_SHORT,
                                        stride, offset);
    }

    void define_vertex_attribute_int32_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, GL_INT,
                                        stride, offset);
    }

    int32_t get_size_of_type(GLenum type)
    {
        switch (type)
        {
        case GL_BYTE:
            return sizeof(GLbyte);
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
