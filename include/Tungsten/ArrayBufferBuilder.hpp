//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include <GL/glew.h>
#include "ArrayBuffer.hpp"

namespace Tungsten
{
    template <typename Item>
    class ArrayBufferBuilder
    {
    public:
        explicit ArrayBufferBuilder(ArrayBuffer<Item>& array)
            : ArrayBufferBuilder(array, array.vertexes.size())
        {}

        explicit ArrayBufferBuilder(ArrayBuffer<Item>& array,
                                    size_t base_index)
            : m_array(array),
              m_base_index(base_index)
        {}

        void reserve_vertexes(size_t count)
        {
            m_array.vertexes.reserve(m_base_index + count);
        }

        void add_vertex(const Item& vertex)
        {
            m_array.vertexes.push_back(vertex);
        }

        [[nodiscard]]
        Item& vertex(size_t index)
        {
            return m_array.vertexes[index];
        }

        void reserve_indexes(size_t count)
        {
            m_array.indexes.reserve(m_array.indexes.size() + count);
        }

        void add_index(uint16_t a)
        {
            m_array.indexes.push_back(a + m_base_index);
        }

        void add_indexes(uint16_t a, uint16_t b, uint16_t c)
        {
            m_array.indexes.push_back(a + m_base_index);
            m_array.indexes.push_back(b + m_base_index);
            m_array.indexes.push_back(c + m_base_index);
        }

    private:
        ArrayBuffer <Item>& m_array;
        uint32_t m_base_index = 0;
    };
}
