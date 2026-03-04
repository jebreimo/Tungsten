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
    void VertexArrayObject::bind() const
    {
        bind_vertex_array(vao_.id());
    }

    VertexArrayObject::operator bool() const
    {
        return bool(vao_);
    }

    uint32_t VertexArrayObject::id() const
    {
        return vao_.id();
    }

    const std::vector<VertexAttribute>& VertexArrayObject::attributes() const
    {
        return attributes_;
    }

    size_t VertexArrayObject::num_buffers() const
    {
        uint32_t buffer_id = 0;
        size_t buffer_count = 0;
        for (const auto& attribute : attributes_)
        {
            if (attribute.buffer_id != buffer_id)
            {
                buffer_id = attribute.buffer_id;
                ++buffer_count;
            }
        }
        return buffer_count;
    }
} // Tungsten
