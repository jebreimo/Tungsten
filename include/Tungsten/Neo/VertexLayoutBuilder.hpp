//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include "VertexLayout.hpp"

namespace Tungsten
{
    class VertexLayoutBuilder
    {
    public:
        VertexLayoutBuilder& add_attribute(AttributeSemantic semantic);

        /**
         * @note offset_in_stream is set to 0 unless @a index is the same
         *  as before.
         */
        VertexLayoutBuilder& set_stream_index(uint8_t index);

        VertexLayoutBuilder& set_data_type(VertexAttributeDataType type);

        VertexLayoutBuilder& set_component_count(uint8_t count);

        VertexLayoutBuilder& set_normalized(bool normalized);

        VertexLayoutBuilder& set_offset_in_stream(uint16_t offset);

        [[nodiscard]] VertexLayout build() const;

    private:
        static VertexAttribute get_default_attribute(AttributeSemantic semantic);

        VertexAttribute& current();

        std::vector<VertexAttribute> layout_;
    };
}
