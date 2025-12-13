//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/VertexArrayObject.hpp"

namespace Tungsten
{
    VertexArrayObject::VertexArrayObject(bool create_now)
    {
        if (create_now)
            create();
    }

    VertexArrayObject::~VertexArrayObject() = default;

    VertexArrayObject::VertexArrayObject(VertexArrayObject&& rhs) noexcept
        : vao(std::move(rhs.vao)),
          vertex_buffer(std::move(rhs.vertex_buffer)),
          element_buffer(std::move(rhs.element_buffer)),
          element_count(rhs.element_count)
    {
        rhs.element_count = 0;
    }

    VertexArrayObject& VertexArrayObject::operator=(VertexArrayObject&& rhs) noexcept
    {
        vao = std::move(rhs.vao);
        vertex_buffer = std::move(rhs.vertex_buffer);
        element_buffer = std::move(rhs.element_buffer);
        std::swap(element_count, rhs.element_count);
        return *this;
    }

    void VertexArrayObject::create()
    {
        vao = generate_vertex_array();
        std::array buffers = {&vertex_buffer, &element_buffer};
        generate_buffers(buffers);
        bind_vertex_array(vao);
        bind_buffer(BufferTarget::ARRAY, vertex_buffer);
        bind_buffer(BufferTarget::ELEMENT_ARRAY, element_buffer);
    }

    void VertexArrayObject::bind() const
    {
        bind_vertex_array(vao);
    }
} // Tungsten
