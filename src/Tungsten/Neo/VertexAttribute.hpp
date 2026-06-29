//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "Tungsten/Gl/GlTypes.hpp"

namespace Tungsten
{
    // The semantic role a vertex attribute plays, independent of the shader
    // location it ends up bound to. A VertexLayout pairs each semantic with a
    // location when it resolves against a ShaderProgram, so meshes describe
    // *what* an attribute is (a position, a normal, ...) rather than hard-coding
    // *where* it goes. The indexed members (TEX_COORD_0/_1, COLOR_0) follow the
    // glTF multi-channel convention.
    enum class AttributeSemantic : uint8_t
    {
        POSITION,
        NORMAL,
        TANGENT,
        BITANGENT,
        TEX_COORD_0,
        TEX_COORD_1,
        COLOR_0,
        BONE_INDICES,
        BONE_WEIGHTS
    };

    // One attribute within a VertexLayout: which semantic it carries, which
    // vertex stream (buffer binding) it is read from, and how to interpret the
    // bytes at offset_in_stream within that stream's vertex.
    struct VertexAttribute
    {
        AttributeSemantic semantic = AttributeSemantic::POSITION;
        uint8_t stream_index = 0;
        VertexAttributeDataType data_type = VertexAttributeDataType::FLOAT;
        uint8_t component_count = 0;
        bool normalized = false;
        uint16_t offset_in_stream = 0;

        bool operator==(const VertexAttribute&) const = default;
    };
} // Tungsten