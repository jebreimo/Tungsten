//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/VertexArrayObjectBuilder.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    VertexArrayObjectBuilder::VertexArrayObjectBuilder() = default;

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_float(uint32_t attribute_location,
                                        int32_t count)
    {
        definitions_.push_back({attribute_location, count, VertexAttributeType::FLOAT});
        return *this;
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_uint32(uint32_t attribute_location,
                                         int32_t count)
    {
        definitions_.push_back({attribute_location, count, VertexAttributeType::UINT32});
        return *this;
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_int32(uint32_t attribute_location,
                                        int32_t count)
    {
        definitions_.push_back({attribute_location, count, VertexAttributeType::INT32});
        return *this;
    }

    VertexArrayObjectBuilder& VertexArrayObjectBuilder::add_stride(int32_t byte_count)
    {
        if (byte_count == 0)
            return *this;
        if (byte_count < 0)
            TUNGSTEN_THROW("Stride byte count must be positive.");
        definitions_.push_back({0, byte_count, VertexAttributeType::NONE});
        return *this;
    }

    VertexArrayObject VertexArrayObjectBuilder::build() const
    {
        VertexArrayObject vbo;
        size_t offset = 0;
        const auto stride = compute_stride();
        for (const auto& def : definitions_)
        {
            if (offset >= stride)
                TUNGSTEN_THROW("Vertex attribute offset exceeds stride.");

            if (def.type == VertexAttributeType::NONE)
            {
                offset += def.count;
                continue;
            }

            define_vertex_attribute_pointer(def.location, def.count,
                                            def.type, stride, offset);
            enable_vertex_attribute(def.location);
            offset += get_size_of_type(def.type) * def.count;
        }
        return vbo;
    }

    int32_t VertexArrayObjectBuilder::get_byte_size(const Definition& definition)
    {
        if (definition.type == VertexAttributeType::NONE)
            return definition.count;
        return get_size_of_type(definition.type) * definition.count;
    }

    int32_t VertexArrayObjectBuilder::compute_stride() const
    {
        int32_t stride = 0;
        for (const auto& def : definitions_)
            stride += get_byte_size(def);
        return stride;
    }
}
