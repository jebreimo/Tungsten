//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/VertexArray.hpp"

#include <algorithm>

#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        // The end of whichever of the stream's attributes reaches furthest
        // into the vertex. Explicit offsets can therefore leave gaps between
        // attributes; padding *after* the last one is not expressible.
        uint32_t derive_stride(const VertexLayout& layout, uint8_t stream)
        {
            uint32_t stride = 0;
            for (const auto& attribute : layout.attributes)
            {
                if (attribute.stream_index != stream)
                    continue;
                stride = std::max(stride, uint32_t(attribute.offset_in_stream)
                                          + byte_size(attribute));
            }
            return stride;
        }
    }

    VertexArrayHandle make_vertex_array(std::span<const uint32_t> buffer_ids,
                                        const VertexLayout& layout)
    {
        for (const auto& attribute : layout.attributes)
        {
            if (attribute.stream_index >= buffer_ids.size())
            {
                TUNGSTEN_THROW("The layout reads a vertex stream there is no"
                    " buffer for.");
            }
        }

        BufferRestorer array_restorer(BufferTarget::ARRAY);

        VertexArrayHandle vao = generate_vertex_array();
        bind_vertex_array(vao.id());

        // Stream by stream rather than attribute by attribute, so each buffer
        // is bound once however many attributes are read from it.
        for (size_t stream = 0; stream < buffer_ids.size(); ++stream)
        {
            if (buffer_ids[stream] == 0)
                TUNGSTEN_THROW("Buffer ID must be non-zero.");

            const auto stride = derive_stride(layout, uint8_t(stream));
            bind_buffer(BufferTarget::ARRAY, buffer_ids[stream]);
            for (const auto& attribute : layout.attributes)
            {
                if (attribute.stream_index != stream)
                    continue;
                const auto location = attribute_location(attribute.semantic);
                define_vertex_attribute_pointer(
                    location, attribute.component_count, attribute.data_type,
                    int32_t(stride), attribute.offset_in_stream,
                    attribute.normalized);
                enable_vertex_attribute(location);
            }
        }
        return vao;
    }

    VertexArrayHandle make_vertex_array(uint32_t buffer_id,
                                        const VertexLayout& layout)
    {
        const uint32_t buffer_ids[] = {buffer_id};
        return make_vertex_array(buffer_ids, layout);
    }
} // Tungsten
