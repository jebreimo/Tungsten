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

        VertexArray() = default;

        static VertexArray create(GLenum usage = GL_STATIC_DRAW)
        {
            VertexArray array;
            array.vertex_array_ = generate_vertex_array();
            array.vertexes = Buffer<Element>(GL_ARRAY_BUFFER, usage);
            array.indexes = Buffer<uint16_t>(GL_ELEMENT_ARRAY_BUFFER, usage);
            return array;
        }

        static VertexArray create_and_bind(GLenum usage = GL_STATIC_DRAW)
        {
            VertexArray array = create(usage);
            array.bind();
            array.vertexes.bind();
            array.indexes.bind();
            return array;
        }

        void bind() const
        {
            bind_vertex_array(vertex_array_);
        }

    private:
        VertexArrayHandle vertex_array_;
    };
}
