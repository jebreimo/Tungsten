//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/VertexArrayObjectBuilder.hpp"

#include <__ranges/reverse_view.h>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    VertexArrayObjectBuilder::VertexArrayObjectBuilder() = default;

    VertexArrayObjectBuilder& VertexArrayObjectBuilder::bind_buffer(uint32_t buffer_id)
    {
        if (buffer_id == 0)
            TUNGSTEN_THROW("Buffer ID must be non-zero.");
        current_buffer_id_ = buffer_id;
        return *this;
    }

    VertexArrayObjectBuilder& VertexArrayObjectBuilder::add(
        const VertexAttributeDefinition& definition)
    {
        if (current_buffer_id_ == 0)
            TUNGSTEN_THROW("Buffer must be bound before adding attributes.");
        if (definition.location == UINT32_MAX && definition.dataType != VertexAttributeDataType::NONE)
            TUNGSTEN_THROW("Attribute location must be less than UINT32_MAX.");
        if (definition.count == 0 || definition.count > UINT8_MAX)
            TUNGSTEN_THROW("Attribute count must be between 1 and 255.");
        definitions_.push_back({
            current_buffer_id_, definition.location, definition.count,
            definition.dataType, definition.type
        });
        return *this;
    }

    VertexArrayObjectBuilder& VertexArrayObjectBuilder::add(
        std::span<const VertexAttributeDefinition> definitions)
    {
        for (auto& def : definitions)
            add(def);
        return *this;
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add(uint32_t attribute_location, VertexAttributeType type)
    {
        auto [value_type, count] = get_data_type_and_count(type);
        add(VertexAttributeDefinition(attribute_location, value_type, count, type));
        return *this;
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_float(uint32_t attribute_location,
                                        uint32_t count)
    {
        return add(VertexAttributeDefinition(attribute_location,
                                             VertexAttributeDataType::FLOAT,
                                             count,
                                             VertexAttributeType::CUSTOM));
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_uint32(uint32_t attribute_location,
                                         uint32_t count)
    {
        return add(VertexAttributeDefinition(attribute_location,
                                             VertexAttributeDataType::UINT32,
                                             count,
                                             VertexAttributeType::CUSTOM));
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_int32(uint32_t attribute_location,
                                        uint32_t count)
    {
        return add(VertexAttributeDefinition(attribute_location,
                                             VertexAttributeDataType::INT32,
                                             count,
                                             VertexAttributeType::CUSTOM));
    }

    VertexArrayObjectBuilder&
    VertexArrayObjectBuilder::add_padding(uint32_t byte_count)
    {
        return add(VertexAttributeDefinition(UINT32_MAX,
                                             VertexAttributeDataType::NONE,
                                             byte_count,
                                             VertexAttributeType::CUSTOM
        ));
    }

    VertexArrayObjectBuilder& VertexArrayObjectBuilder::set_divisor(uint32_t divisor)
    {
        if (definitions_.empty())
            TUNGSTEN_THROW("No attributes defined to set divisor for.");
        const uint32_t last_buffer_id = definitions_.back().buffer_id;
        for (auto& def : std::ranges::views::reverse(definitions_))
        {
            if (def.buffer_id != last_buffer_id)
                break;
            if (def.type != VertexAttributeDataType::NONE)
                def.divisor = divisor;
        }
        return *this;
    }

    VertexArrayObject VertexArrayObjectBuilder::build() const
    {
        VertexArrayHandle vao = generate_vertex_array();
        const auto strides = compute_strides();
        std::vector<VertexAttribute> attributes;
        for (size_t i = 0; i < definitions_.size(); ++i)
        {
            const auto& def = definitions_[i];
            if (def.type == VertexAttributeDataType::NONE)
                continue;
            if (strides[i] > UINT8_MAX)
                TUNGSTEN_THROW("Stride exceeds maximum of 255 bytes.");

            attributes.push_back({
                {
                    def.location,
                    def.type,
                    uint8_t(def.count),
                    def.attribute_type
                },
                def.buffer_id,
                uint8_t(def.divisor),
                uint8_t(0),
                uint8_t(strides[i]),
            });
            define_vertex_attribute_pointer(def.location, def.count,
                                            def.type, strides[i], 0);
            enable_vertex_attribute(def.location);
        }
        return {std::move(vao), std::move(attributes)};
    }

    std::vector<int32_t> VertexArrayObjectBuilder::compute_strides() const
    {
        std::vector<int32_t> strides;
        strides.reserve(definitions_.size());
        int32_t current_stride = 0;
        uint32_t current_buffer_id = 0;
        for (size_t i = 0; i < definitions_.size(); ++i)
        {
            if (definitions_[i].buffer_id != current_buffer_id)
            {
                for (size_t j = i; j-- > 0 && definitions_[j].buffer_id == current_buffer_id;)
                    strides.push_back(current_stride);
                current_stride = get_byte_size(definitions_[i]);
                current_buffer_id = definitions_[i].buffer_id;
            }
            else
            {
                current_stride += get_byte_size(definitions_[i]);
            }
        }

        for (size_t j = definitions_.size();
             j-- > 0 && definitions_[j].buffer_id == current_buffer_id;)
            strides.push_back(current_stride);

        return strides;
    }

    int32_t VertexArrayObjectBuilder::get_byte_size(const Definition& definition)
    {
        if (definition.type == VertexAttributeDataType::NONE)
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
