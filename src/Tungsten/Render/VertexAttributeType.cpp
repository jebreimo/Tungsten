//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/VertexAttributeType.hpp"

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
            TUNGSTEN_THROW("Can't determine data type for "
                + std::to_string(int(type)));
        }
    }

    std::string get_default_name(VertexAttributeType type)
    {
        switch (type)
        {
        case VertexAttributeType::POSITION_2F:
        case VertexAttributeType::POSITION_3F:
            return "position";
        case VertexAttributeType::NORMAL_2F:
        case VertexAttributeType::NORMAL_3F:
            return "normal";
        case VertexAttributeType::TEX_COORD_2F:
            return "texture";
        case VertexAttributeType::COLOR_3F:
        case VertexAttributeType::COLOR_4F:
            return "color";
        case VertexAttributeType::TANGENT_3F:
            return "tangent";
        case VertexAttributeType::BITANGENT_3F:
            return "bitangent";
        default:
            return "unknown";
        }
    }
}
