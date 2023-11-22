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

        ArrayBufferBuilder& reserve_vertexes(size_t count)
        {
            array_.vertexes.reserve(base_index_ + count);
            return *this;
        }

        ArrayBufferBuilder& add_vertex(const Item& vertex)
        {
            array_.vertexes.push_back(vertex);
            return *this;
        }

        [[nodiscard]]
        Item& vertex(size_t index)
        {
            return array_.vertexes[index];
        }

        ArrayBufferBuilder& reserve_indexes(size_t count)
        {
            array_.indexes.reserve(array_.indexes.size() + count);
            return *this;
        }

        ArrayBufferBuilder& add_index(uint16_t a)
        {
            array_.indexes.push_back(a + base_index_);
            return *this;
        }

        ArrayBufferBuilder& add_indexes(uint16_t a, uint16_t b, uint16_t c)
        {
            array_.indexes.push_back(a + base_index_);
            array_.indexes.push_back(b + base_index_);
            array_.indexes.push_back(c + base_index_);
            return *this;
        }
    private:
        ArrayBuffer<Item>& array_;
        uint32_t base_index_ = 0;
    };
}
