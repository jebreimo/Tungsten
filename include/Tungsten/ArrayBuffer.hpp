//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-01-25.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <vector>
#include "GlBuffer.hpp"

namespace Tungsten
{
    template <typename ItemType>
    struct ArrayBuffer
    {
    public:
        using Item = ItemType;

        std::vector<Item> vertexes;
        std::vector<uint16_t> indexes;

        [[nodiscard]]
        std::pair<const void*, size_t> array_buffer() const
        {
            return {vertexes.data(), vertexes.size() * sizeof(Item)};
        }

        [[nodiscard]]
        std::pair<const uint16_t*, size_t> index_buffer() const
        {
            return {indexes.data(), indexes.size() * sizeof(uint16_t)};
        }
    };

    template <typename Item>
    void set_buffers(GLuint array_buffer, GLuint element_array_buffer,
                     const ArrayBuffer<Item>& array,
                     GLenum usage = GL_STATIC_DRAW)
    {
        Tungsten::bind_buffer(GL_ARRAY_BUFFER, array_buffer);
        auto [v_buf, v_size] = array.array_buffer();
        Tungsten::set_buffer_data(GL_ARRAY_BUFFER, v_size, v_buf, usage);
        Tungsten::bind_buffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
        auto [i_buf, i_size] = array.index_buffer();
        Tungsten::set_buffer_data(GL_ELEMENT_ARRAY_BUFFER, i_size, i_buf, usage);
    }
}
