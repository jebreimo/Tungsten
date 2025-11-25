//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-07-31.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "TungstenException.hpp"
#include "VertexArray.hpp"

namespace Tungsten
{
    template <typename Element>
    class VertexArrayBuilder
    {
    public:
        VertexArrayBuilder& add_float(uint32_t attribute_location, int32_t count)
        {
            definitions_.push_back({attribute_location, count, GL_FLOAT});
            return *this;
        }

        VertexArrayBuilder& add_int16(uint32_t attribute_location, int32_t count)
        {
            definitions_.push_back({attribute_location, count, GL_SHORT});
            return *this;
        }

        VertexArrayBuilder& add_int32(uint32_t attribute_location, int32_t count)
        {
            definitions_.push_back({attribute_location, count, GL_INT});
            return *this;
        }

        VertexArray<Element> build() const
        {
            VertexArray<Element> array = VertexArray<Element>::create_and_bind();
            size_t offset = 0;
            for (const auto& def : definitions_)
            {
                constexpr auto stride = sizeof(Element);
                if (offset >= stride)
                    TUNGSTEN_THROW("Vertex attribute offset exceeds stride.");

                define_vertex_attribute_pointer(def.location, def.count,
                                                def.type, stride, offset);
                enable_vertex_attribute(def.location);
                offset += get_size_of_type(def.type) * def.count;
            }
            return array;
        }

    private:
        struct Definition
        {
            uint32_t location;
            int32_t count;
            GLenum type;
        };

        std::vector<Definition> definitions_;
    };
}
