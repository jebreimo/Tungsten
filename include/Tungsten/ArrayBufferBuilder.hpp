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
#include "TungstenException.hpp"

namespace Tungsten
{
    template <typename Item>
    class ArrayBufferBuilder
    {
    public:
        using ArrayBuffer = ArrayBuffer<Item>;

        explicit ArrayBufferBuilder(ArrayBuffer& array)
            : ArrayBufferBuilder(array, array.vertexes.size())
        {}

        explicit ArrayBufferBuilder(ArrayBuffer& array,
                                    size_t base_vertex)
            : array_(array),
              base_vertex_(base_vertex),
              base_index_(array.indexes.size()),
              impl_(Appender())
        {}

        explicit ArrayBufferBuilder(ArrayBuffer& array,
                                    size_t base_vertex,
                                    size_t vertex_offset,
                                    size_t vertex_size,
                                    size_t index_offset,
                                    size_t index_size)
            : array_(array),
              base_vertex_(base_vertex),
              base_index_(index_offset),
              impl_(Setter{vertex_offset, vertex_size,
                           index_offset, index_size})
        {}

        ArrayBufferBuilder& reserve_vertexes(size_t count)
        {
            Wrapper{impl_}.reserve_vertexes(array_, base_vertex_ + count);
            return *this;
        }

        ArrayBufferBuilder& add_vertex(const Item& vertex)
        {
            Wrapper{impl_}.add_vertex(array_, vertex);
            return *this;
        }

        [[nodiscard]]
        Item& vertex(size_t index)
        {
            return array_.vertexes[index + base_vertex_];
        }

        ArrayBufferBuilder& reserve_indexes(size_t count)
        {
            Wrapper{impl_}.reserve_indexes(array_, base_index_ + count);
            return *this;
        }

        ArrayBufferBuilder& add_index(uint16_t a)
        {
            Wrapper{impl_}.add_index(array_, a + base_vertex_);
            return *this;
        }

        ArrayBufferBuilder& add_indexes(uint16_t a, uint16_t b, uint16_t c)
        {
            Wrapper wrapper{impl_};
            wrapper.add_index(array_, a + base_vertex_);
            wrapper.add_index(array_, b + base_vertex_);
            wrapper.add_index(array_, c + base_vertex_);
            return *this;
        }
    private:
        struct Appender
        {
            void reserve_vertexes(ArrayBuffer& buffer, size_t count)
            {
                buffer.vertexes.reserve(count);
            }

            void add_vertex(ArrayBuffer& buffer, const Item& item)
            {
                buffer.push_back(item);
            }

            void reserve_indexes(ArrayBuffer& buffer, size_t count)
            {
                buffer.indexes.reserve(count);
            }

            void add_index(ArrayBuffer& buffer, uint16_t index)
            {
                buffer.indexes.push_back(index);
            }
        };

        struct Setter
        {
            Setter(size_t vertex_offset,
                   size_t vertex_size,
                   size_t index_offset,
                   size_t index_size)
               : vertex_pos(vertex_offset),
                 vertex_max(vertex_offset + vertex_size),
                 index_pos(index_offset),
                 index_max(index_offset + index_size)
            {}

            void reserve_vertexes(ArrayBuffer&, size_t)
            {}

            void add_vertex(ArrayBuffer& buffer, const Item& item)
            {
                if (vertex_pos == vertex_max)
                    TUNGSTEN_THROW("Maximum number of vertexes has been reached");
                buffer[vertex_pos++] = item;
            }

            void reserve_indexes(ArrayBuffer&, size_t)
            {}

            void add_index(ArrayBuffer& buffer, uint16_t index)
            {
                if (index_pos == index_max)
                    TUNGSTEN_THROW("Maximum number of indexes has been reached");
                buffer[index_pos++] = index;
            }

            size_t vertex_pos;
            size_t vertex_max;
            size_t index_pos;
            size_t index_max;
        };

        using Impl = std::variant<Appender, Setter>;

        struct Wrapper
        {
            template <class... Ts>
            struct overload : Ts... {using Ts::operator()...;};
            template <class... Ts>
            overload(Ts...) -> overload<Ts...>;

            void reserve_vertexes(ArrayBuffer& buffer, size_t count)
            {
                std::visit(overload{
                    [&](const Appender& a){a.reserve_vertexes(buffer, count);},
                    [&](const Setter& s){s.reserve_vertexes(buffer, count);},
                }, impl);
            }

            void add_vertex(ArrayBuffer& buffer, Item& item)
            {
                std::visit(overload{
                    [&](const Appender& a){a.add_vertexes(buffer, item);},
                    [&](const Setter& s){s.add_vertexes(buffer, item);},
                }, impl);
            }

            void reserve_indexes(ArrayBuffer& buffer, size_t count)
            {
                std::visit(overload{
                    [&](const Appender& a){a.reserve_indexes(buffer, count);},
                    [&](const Setter& s){s.reserve_indexes(buffer, count);},
                }, impl);
            }

            void add_index(ArrayBuffer& buffer, Item& item)
            {
                std::visit(overload{
                    [&](const Appender& a){a.add_indexes(buffer, item);},
                    [&](const Setter& s){s.add_indexes(buffer, item);},
                }, impl);
            }

            Impl& impl;
        };
        ArrayBuffer& array_;
        uint32_t base_vertex_ = 0;
        uint32_t base_index_ = 0;
        Impl impl_;
    };
}
