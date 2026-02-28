//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "VertexAttributeType.hpp"

namespace Tungsten
{
    struct VertexAttributeDefinition
    {
        uint32_t location = UINT32_MAX;
        VertexAttributeDataType data_type = VertexAttributeDataType::FLOAT;
        uint8_t count = 0;
        VertexAttributeType type = VertexAttributeType::CUSTOM;
        std::string name;

        VertexAttributeDefinition() = default;

        VertexAttributeDefinition(uint32_t location, VertexAttributeType type)
            : location(location),
              type(type),
              name(get_default_name(type))
        {
            std::tie(data_type, count) = get_data_type_and_count(type);
        }

        VertexAttributeDefinition(uint32_t location,
                                  VertexAttributeDataType data_type,
                                  uint8_t count,
                                  std::string name,
                                  VertexAttributeType type = VertexAttributeType::CUSTOM)
            : location(location),
              data_type(data_type),
              count(count),
              type(type),
              name(std::move(name))
        {
        }
    };
}
