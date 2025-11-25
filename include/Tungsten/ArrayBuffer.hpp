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
#include "VertexArray.hpp"

namespace Tungsten
{
    template <typename ItemType>
    struct ArrayBuffer
    {
        std::vector<ItemType> vertexes;
        std::vector<uint16_t> indexes;
    };

    template <typename Item>
    void set_buffers(uint32_t array_buffer, uint32_t element_array_buffer,
                     const ArrayBuffer<Item>& array,
                     GLenum usage = GL_STATIC_DRAW)
    {
        bind_buffer(GL_ARRAY_BUFFER, array_buffer);
        auto [v_buf, v_size] = array.array_buffer();
        Tungsten::set_buffer_data(GL_ARRAY_BUFFER, v_size, v_buf, usage);
        bind_buffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
        auto [i_buf, i_size] = array.index_buffer();
        Tungsten::set_buffer_data(GL_ELEMENT_ARRAY_BUFFER, i_size, i_buf, usage);
    }

    template <typename Item>
    void set_buffers(VertexArray<Item>& vertex_array,
                     const ArrayBuffer<Item>& array,
                     GLenum usage = GL_STATIC_DRAW)
    {
        vertex_array.bind();
        vertex_array.vertexes.set_data(array.vertexes);
        vertex_array.indexes.set_data(array.indexes);
    }
}
