//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>

namespace Tungsten
{
    template <typename Item>
    struct MeshData
    {
        using ItemType = Item;
        std::vector<Item> vertices;
        std::vector<uint16_t> indices;
    };

    template <typename Item, typename Index = uint16_t>
    class MeshDataBuilder
    {
    public:
        explicit MeshDataBuilder(MeshData<Item>& array)
            : MeshDataBuilder(array, array.vertices.size())
        {
        }

        explicit MeshDataBuilder(MeshData<Item>& array,
                                       Index base_index)
            : array_(array),
              base_index_(base_index)
        {
        }

        MeshDataBuilder& reserve_vertexes(Index count)
        {
            array_.vertices.reserve(base_index_ + count);
            return *this;
        }

        MeshDataBuilder& add_vertex(const Item& vertex)
        {
            array_.vertices.push_back(vertex);
            return *this;
        }

        [[nodiscard]]
        Item& vertex(Index index)
        {
            return array_.vertices[index];
        }

        MeshDataBuilder& reserve_indexes(size_t count)
        {
            array_.indices.reserve(array_.indices.size() + count);
            return *this;
        }

        MeshDataBuilder& add_index(Index a)
        {
            array_.indices.push_back(a + base_index_);
            return *this;
        }

        MeshDataBuilder& add_indexes(Index a, Index b, Index c)
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

        [[nodiscard]] MeshData<Item>& vertex_array()
        {
            return array_;
        }
    private:
        MeshData<Item>& array_;
        Index base_index_ = 0;
    };
}
