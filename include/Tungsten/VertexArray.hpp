//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-11-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Buffer.hpp"
#include "GlVertexArray.hpp"
#include "GlVertexArray.hpp"

namespace Tungsten
{
    template <typename Element>
    class VertexArray
    {
    public:
        Buffer<Element> vertexes;
        Buffer<uint16_t> indexes;

        VertexArray()
            : vertexes(GL_ARRAY_BUFFER),
              indexes(GL_ELEMENT_ARRAY_BUFFER),
              vertex_array_(generate_vertex_array())
        {}

        void bind() const
        {
            bind_vertex_array(vertex_array_);
        }

        void define_float_pointer(GLuint attribute_location,
                                  GLint size,
                                  GLsizei stride,
                                  size_t offset = 0)
        {
            bind();
            vertexes.bind();
            define_vertex_attribute_float_pointer(attribute_location,
                                                  size, stride, offset);
        }

        void define_int32_pointer(GLuint attribute_location,
                                  GLint size,
                                  GLsizei stride,
                                  size_t offset = 0)
        {
            bind();
            vertexes.bind();
            define_vertex_attribute_int32_pointer(attribute_location,
                                                  size, stride, offset);
        }
    private:
        VertexArrayHandle vertex_array_;
    };
}
