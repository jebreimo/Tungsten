//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/VertexLayoutBuilder.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    VertexLayoutBuilder& VertexLayoutBuilder::add_attribute(AttributeSemantic semantic)
    {
        auto attr = get_default_attribute(semantic);
        if (!layout_.empty())
        {
            const auto& prev = layout_.back();
            attr.stream_index = prev.stream_index;
            attr.offset_in_stream = prev.offset_in_stream + byte_size(prev);
        }
        layout_.push_back(attr);
        return *this;
    }

    VertexLayoutBuilder& VertexLayoutBuilder::set_stream_index(uint8_t index)
    {
        auto& c = current();
        if (c.stream_index != index)
        {
            c.stream_index = index;
            c.offset_in_stream = 0;
        }
        return *this;
    }

    VertexLayoutBuilder& VertexLayoutBuilder::set_data_type(VertexAttributeDataType type)
    {
        current().data_type = type;
        return *this;
    }

    VertexLayoutBuilder& VertexLayoutBuilder::set_component_count(uint8_t count)
    {
        current().component_count = count;
        return *this;
    }

    VertexLayoutBuilder& VertexLayoutBuilder::set_normalized(bool normalized)
    {
        current().normalized = normalized;
        return *this;
    }

    VertexLayoutBuilder& VertexLayoutBuilder::set_offset_in_stream(uint16_t offset)
    {
        current().offset_in_stream = offset;
        return *this;
    }

    VertexLayout VertexLayoutBuilder::build() const
    {
        return VertexLayout(layout_);
    }

    VertexAttribute VertexLayoutBuilder::get_default_attribute(AttributeSemantic semantic)
    {
        switch (semantic)
        {
        case AttributeSemantic::POSITION:
        case AttributeSemantic::NORMAL:
        case AttributeSemantic::TANGENT:
        case AttributeSemantic::BITANGENT:
        case AttributeSemantic::COLOR_0:
            return {
                .semantic = semantic,
                .component_count = 3
            };
        case AttributeSemantic::TEX_COORD_0:
        case AttributeSemantic::TEX_COORD_1:
            return {
                .semantic = semantic,
                .component_count = 2
            };
        case AttributeSemantic::BONE_INDICES:
        case AttributeSemantic::BONE_WEIGHTS:
        default:
            TUNGSTEN_THROW("Bone indices and weights are unsupported");
        }
    }

    VertexAttribute& VertexLayoutBuilder::current()
    {
        if (layout_.empty())
            TUNGSTEN_THROW("No attributes have been added.");
        return layout_.back();
    }
}
