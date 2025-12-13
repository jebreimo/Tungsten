//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlTypes.hpp"
#include "VertexArrayObject.hpp"

namespace Tungsten
{
    class VertexArrayObjectBuilder
    {
    public:
        VertexArrayObjectBuilder();

        VertexArrayObjectBuilder& add_float(uint32_t attribute_location, int32_t count);

        VertexArrayObjectBuilder& add_uint32(uint32_t attribute_location, int32_t count);

        VertexArrayObjectBuilder& add_int32(uint32_t attribute_location, int32_t count);

        [[nodiscard]] VertexArrayObject build() const;

    private:
        struct Definition
        {
            uint32_t location;
            int32_t count;
            VertexAttributeType type;
        };

        [[nodiscard]] static int32_t get_byte_size(const Definition& definition);

        [[nodiscard]] int32_t compute_stride() const;

        std::vector<Definition> definitions_;
    };
}
