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
            : array_(array),
              base_index_(base_index)
        {}

        void reserve_vertexes(size_t count)
        {
            array_.vertexes.reserve(base_index_ + count);
        }

        void add_vertex(const Item& vertex)
        {
            array_.vertexes.push_back(vertex);
        }

        [[nodiscard]]
        Item& vertex(size_t index)
        {
            return array_.vertexes[index];
        }

        void reserve_indexes(size_t count)
        {
            array_.indexes.reserve(array_.indexes.size() + count);
        }

        void add_index(uint16_t a)
        {
            array_.indexes.push_back(a + base_index_);
        }

        void add_indexes(uint16_t a, uint16_t b, uint16_t c)
        {
            array_.indexes.push_back(a + base_index_);
            array_.indexes.push_back(b + base_index_);
            array_.indexes.push_back(c + base_index_);
        }
    private:
        ArrayBuffer<Item>& array_;
        uint32_t base_index_ = 0;
    };
}
