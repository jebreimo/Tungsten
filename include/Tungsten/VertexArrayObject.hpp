//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include "Gl/GlVertexArray.hpp"

namespace Tungsten
{
    enum class VertexAttributeType : uint8_t
    {
        POSITION_2F,
        POSITION_3F,
        NORMAL_2F,
        NORMAL_3F,
        TEX_COORD_2F,
        COLOR_3F,
        COLOR_4F,
        TANGENT_3F,
        BITANGENT_3F,
        CUSTOM
    };

    [[nodiscard]] std::pair<VertexAttributeDataType, uint32_t>
    get_data_type_and_count(VertexAttributeType type);

    struct VertexAttributeDefinition
    {
        uint32_t location = UINT32_MAX;
        VertexAttributeDataType dataType = VertexAttributeDataType::FLOAT;
        uint8_t count = 0;
        VertexAttributeType type = VertexAttributeType::CUSTOM;

        VertexAttributeDefinition() = default;

        VertexAttributeDefinition(uint32_t location, VertexAttributeType type)
            : location(location),
              type(type)
        {
            std::tie(dataType, count) = get_data_type_and_count(type);
        }

        VertexAttributeDefinition(uint32_t location,
                                  VertexAttributeDataType data_type,
                                  uint8_t count,
                                  VertexAttributeType type = VertexAttributeType::CUSTOM)
            : location(location),
              dataType(data_type),
              count(count),
              type(type)
        {
        }
    };

    struct VertexAttribute
    {
        VertexAttributeDefinition definition;
        uint32_t buffer_id = 0;
        uint8_t divisor = 0;
        uint8_t offset = 0;
        uint8_t stride = 0;
    };

    class VertexArrayObject
    {
    public:
        VertexArrayObject() = default;

        VertexArrayObject(VertexArrayHandle vao,
                           std::vector<VertexAttribute> attributes)
            : vao_(std::move(vao)),
              attributes_(std::move(attributes))
        {
        }

        ~VertexArrayObject() = default;

        VertexArrayObject(const VertexArrayObject&) = delete;

        VertexArrayObject& operator=(const VertexArrayObject&) = delete;

        VertexArrayObject(VertexArrayObject&&) noexcept = default;

        VertexArrayObject& operator=(VertexArrayObject&&) noexcept = default;

        void bind() const;

        [[nodiscard]] explicit operator bool() const;

        [[nodiscard]] uint32_t handle() const;

        [[nodiscard]] const std::vector<VertexAttribute>& attributes() const;

        [[nodiscard]] size_t num_buffers() const;

    private:
        VertexArrayHandle vao_;
        std::vector<VertexAttribute> attributes_;
    };
} // Tungsten
