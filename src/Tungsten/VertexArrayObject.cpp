//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/VertexArrayObject.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    std::pair<VertexAttributeDataType, uint32_t>
    get_data_type_and_count(VertexAttributeType type)
    {
        switch (type)
        {
        case VertexAttributeType::POSITION_2F:
        case VertexAttributeType::NORMAL_2F:
        case VertexAttributeType::TEX_COORD_2F:
            return {VertexAttributeDataType::FLOAT, 2};
        case VertexAttributeType::POSITION_3F:
        case VertexAttributeType::NORMAL_3F:
        case VertexAttributeType::TANGENT_3F:
        case VertexAttributeType::BITANGENT_3F:
        case VertexAttributeType::COLOR_3F:
            return {VertexAttributeDataType::FLOAT, 3};
        case VertexAttributeType::COLOR_4F:
            return {VertexAttributeDataType::FLOAT, 4};
        default:
            TUNGSTEN_THROW("Unsupported type" + std::to_string(int(type)));
        }
    }

    void VertexArrayObject::bind() const
    {
        bind_vertex_array(vao_);
    }

    VertexArrayObject::operator bool() const
    {
        return bool(vao_);
    }

    uint32_t VertexArrayObject::handle() const
    {
        return vao_;
    }

    const std::vector<VertexAttribute>& VertexArrayObject::attributes() const
    {
        return attributes_;
    }

    size_t VertexArrayObject::num_buffers() const
    {
        uint32_t buffer_id = 0;
        size_t buffer_count = 0;
        for (const auto& attribute : attributes_)
        {
            if (attribute.buffer_id != buffer_id)
            {
                buffer_id = attribute.buffer_id;
                ++buffer_count;
            }
        }
        return buffer_count;
    }
} // Tungsten
