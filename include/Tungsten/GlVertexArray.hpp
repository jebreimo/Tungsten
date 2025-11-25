//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlVertexArrayDeleter
    {
        void operator()(uint32_t id) const;
    };

    using VertexArrayHandle = GlHandle<GlVertexArrayDeleter>;

    VertexArrayHandle generate_vertex_array();

    void generate_vertex_arrays(std::span<VertexArrayHandle> vertex_arrays);

    void bind_vertex_array(uint32_t vertex_array);

    void define_vertex_attribute_pointer(uint32_t location,
                                         int32_t size,
                                         GLenum type,
                                         int32_t stride,
                                         size_t offset = 0,
                                         bool normalized = false);

    void define_vertex_attribute_float_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset = 0);

    void define_vertex_attribute_int16_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset = 0);

    void define_vertex_attribute_int32_pointer(uint32_t location,
                                               int32_t size,
                                               int32_t stride,
                                               size_t offset = 0);

    void enable_vertex_attribute(uint32_t location);

    void disable_vertex_attribute(uint32_t location);

    int32_t get_size_of_type(GLenum type);
}
