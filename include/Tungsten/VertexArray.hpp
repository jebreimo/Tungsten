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
    private:
        VertexArrayHandle vertex_array_;
    };
}
