//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Gl/GlTypes.hpp"
#include "VertexArrayObject.hpp"

namespace Tungsten
{
    class VertexArrayObjectBuilder
    {
    public:
        VertexArrayObjectBuilder();

        VertexArrayObjectBuilder& bind_buffer(uint32_t buffer_id);

        VertexArrayObjectBuilder&
        add(const VertexAttributeDefinition& definition);

        VertexArrayObjectBuilder&
        add(std::span<const VertexAttributeDefinition> definitions);

        VertexArrayObjectBuilder&
        add(uint32_t attribute_location, VertexAttributeType type);

        VertexArrayObjectBuilder&
        add_float(uint32_t attribute_location, uint32_t count);

        VertexArrayObjectBuilder&
        add_uint32(uint32_t attribute_location, uint32_t count);

        VertexArrayObjectBuilder&
        add_int32(uint32_t attribute_location, uint32_t count);

        VertexArrayObjectBuilder&
        add_padding(uint32_t byte_count);

        VertexArrayObjectBuilder& set_divisor(uint32_t divisor);

        [[nodiscard]] VertexArrayObject build() const;

    private:
        struct Definition
        {
            uint32_t buffer_id = 0;
            uint32_t location = UINT32_MAX;
            uint8_t count = 0;
            VertexAttributeDataType type = VertexAttributeDataType::FLOAT;
            VertexAttributeType attribute_type = VertexAttributeType::CUSTOM;
            uint32_t divisor = 0;
        };

        [[nodiscard]] static int32_t get_byte_size(const Definition& definition);

        [[nodiscard]] int32_t compute_stride() const;

        [[nodiscard]] std::vector<int32_t> compute_strides() const;

        std::vector<Definition> definitions_;
        uint32_t current_buffer_id_ = 0;
    };
}
