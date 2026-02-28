//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "Gl/GlTypes.hpp"

namespace Tungsten
{
    enum class VertexAttributeType : std::uint8_t
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

    std::string get_default_name(VertexAttributeType type);
}
