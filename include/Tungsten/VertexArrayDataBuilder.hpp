//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>

namespace Tungsten
{
    template <typename Item>
    struct VertexArrayData
    {
        using ItemType = Item;
        std::vector<Item> vertices;
        std::vector<uint16_t> indices;
    };

    template <typename Item, typename Index = uint16_t>
    class VertexArrayDataBuilder
    {
    public:
        explicit VertexArrayDataBuilder(VertexArrayData<Item>& array)
            : VertexArrayDataBuilder(array, array.vertices.size())
        {
        }

        explicit VertexArrayDataBuilder(VertexArrayData<Item>& array,
                                       Index base_index)
            : array_(array),
              base_index_(base_index)
        {
        }

        VertexArrayDataBuilder& reserve_vertexes(Index count)
        {
            array_.vertices.reserve(base_index_ + count);
            return *this;
        }

        VertexArrayDataBuilder& add_vertex(const Item& vertex)
        {
            array_.vertices.push_back(vertex);
            return *this;
        }

        [[nodiscard]]
        Item& vertex(Index index)
        {
            return array_.vertices[index];
        }

        VertexArrayDataBuilder& reserve_indexes(size_t count)
        {
            array_.indices.reserve(array_.indices.size() + count);
            return *this;
        }

        VertexArrayDataBuilder& add_index(Index a)
        {
            array_.indices.push_back(a + base_index_);
            return *this;
        }

        VertexArrayDataBuilder& add_indexes(Index a, Index b, Index c)
        {
            array_.indices.push_back(a + base_index_);
            array_.indices.push_back(b + base_index_);
            array_.indices.push_back(c + base_index_);
            return *this;
        }

        [[nodiscard]] Index max_index() const
        {
            return Index(array_.vertices.size()) - base_index_;
        }

        [[nodiscard]] VertexArrayData<Item>& vertex_array()
        {
            return array_;
        }
    private:
        VertexArrayData<Item>& array_;
        Index base_index_ = 0;
    };
}
