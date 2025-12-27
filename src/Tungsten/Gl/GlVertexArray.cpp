//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlVertexArray.hpp"

#include <vector>

#include "GlTypeConversion.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlVertexArrayDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().delete_vertex_arrays(1, &id);
        THROW_IF_GL_ERROR();
    }

    VertexArrayHandle generate_vertex_array()
    {
        uint32_t id;
        get_ogl_wrapper().gen_vertex_arrays(1, &id);
        THROW_IF_GL_ERROR();
        return VertexArrayHandle(id);
    }

    void generate_vertex_arrays(std::span<VertexArrayHandle> vertex_arrays)
    {
        auto ids = std::vector<uint32_t>(vertex_arrays.size());
        get_ogl_wrapper().gen_vertex_arrays(static_cast<int32_t>(ids.size()), ids.data());
        THROW_IF_GL_ERROR();
        for (size_t i = 0; i < ids.size(); ++i)
            vertex_arrays[i] = VertexArrayHandle(ids[i]);
    }

    void bind_vertex_array(uint32_t vertex_array)
    {
        get_ogl_wrapper().bind_vertex_array(vertex_array);
        THROW_IF_GL_ERROR();
    }

    void enable_vertex_attribute(uint32_t location)
    {
        get_ogl_wrapper().enable_vertex_attrib_array(location);
        THROW_IF_GL_ERROR();
    }

    void disable_vertex_attribute(uint32_t location)
    {
        get_ogl_wrapper().disable_vertex_attrib_array(location);
        THROW_IF_GL_ERROR();
    }

    void define_vertex_attribute_pointer(uint32_t location,
                                         int32_t size,
                                         VertexAttributeType type,
                                         int32_t stride,
                                         size_t offset,
                                         bool normalized)
    {
        get_ogl_wrapper().vertex_attrib_pointer(location, size,
                                              to_ogl_vertex_attribute_type(type),
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
        define_vertex_attribute_pointer(location, size, VertexAttributeType::FLOAT,
                                        stride, offset);
    }

    void define_vertex_attribute_uint16_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, VertexAttributeType::UINT32,
                                        stride, offset);
    }

    void define_vertex_attribute_int32_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset)
    {
        define_vertex_attribute_pointer(location, size, VertexAttributeType::INT32,
                                        stride, offset);
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    int32_t get_size_of_type(VertexAttributeType type)
    {
        switch (type)
        {
        case VertexAttributeType::INT32:
            return sizeof(GLint);
        case VertexAttributeType::UINT32:
            return sizeof(GLuint);
        case VertexAttributeType::FLOAT:
            return sizeof(GLfloat);
        default:
            TUNGSTEN_THROW("Unknown type in get_size_of_type: "
                + std::to_string(static_cast<int>(type)));
        }
    }

    bool is_float_type(VertexAttributeType type)
    {
        return type == VertexAttributeType::FLOAT;
    }
}
