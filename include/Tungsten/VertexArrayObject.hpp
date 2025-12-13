//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include "GlBuffer.hpp"
#include "GlVertexArray.hpp"

namespace Tungsten
{
    class VertexArrayObject
    {
    public:
        VertexArrayHandle vao;
        BufferHandle vertices;
        BufferHandle indices;
        int32_t index_count = 0;

        explicit VertexArrayObject(bool create_now = true);

        ~VertexArrayObject();

        VertexArrayObject(const VertexArrayObject&) = delete;
        VertexArrayObject& operator=(const VertexArrayObject&) = delete;

        VertexArrayObject(VertexArrayObject&&) noexcept;
        VertexArrayObject& operator=(VertexArrayObject&&) noexcept;

        void create();

        void bind() const;

        template <typename ItemType>
        void set_data(std::span<const ItemType> vertices,
                      std::span<const uint16_t> indices,
                      BufferUsage usage = BufferUsage::STATIC_DRAW)
        {
            bind();
            set_buffer_data(BufferTarget::ARRAY,
                            ptrdiff_t(vertices.size() * sizeof(ItemType)),
                            vertices.data(),
                            usage);
            set_buffer_data(BufferTarget::ELEMENT_ARRAY,
                            ptrdiff_t(indices.size() * sizeof(uint16_t)),
                            indices.data(),
                            usage);
            index_count = static_cast<int32_t>(indices.size());
        }
    };
} // Tungsten
