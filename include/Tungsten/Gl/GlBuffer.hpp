//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>
#include "GlHandle.hpp"
#include "GlTypes.hpp"

namespace Tungsten
{
    struct BufferDeleter
    {
        void operator()(uint32_t id) const;
    };

    using BufferHandle = GlHandle<BufferDeleter>;

    BufferHandle generate_buffer();

    void generate_buffers(std::span<BufferHandle> buffers);

    void generate_buffers(std::span<BufferHandle*> buffers);

    void bind_buffer(BufferTarget target, uint32_t buffer);

    void set_buffer_data(BufferTarget target, ptrdiff_t size,
                         const void* data, BufferUsage usage);

    void set_buffer_subdata(BufferTarget target, ptrdiff_t offset,
                            ptrdiff_t size, const void* data);

    void set_element_array_buffer(uint32_t buffer_id,
                                  ptrdiff_t value_count,
                                  const uint16_t* values,
                                  BufferUsage usage);

    [[nodiscard]]
    bool is_buffer(uint32_t buffer);

    [[nodiscard]]
    int32_t get_buffer_size(BufferTarget target);

    [[nodiscard]]
    BufferUsage get_buffer_usage(BufferTarget target);
}
