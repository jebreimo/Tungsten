//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the Zero-Clause BSD License.
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

    BufferHandle generate_buffer(ptrdiff_t size,
                                 BufferUsage usage,
                                 BufferTarget target = BufferTarget::COPY_WRITE);

    void generate_buffers(std::span<BufferHandle> buffers);

    void generate_buffers(std::span<BufferHandle*> buffers);

    void bind_buffer(BufferTarget target, uint32_t buffer);

    // Binds the whole buffer to the indexed binding point `index` of an indexed
    // target (e.g. BufferTarget::UNIFORM), and also to the generic target.
    void bind_buffer_base(BufferTarget target, uint32_t index, uint32_t buffer);

    // Binds the [offset, offset + size) range of the buffer to the indexed
    // binding point `index`. Use to point one UBO's binding at a sub-range
    // (e.g. a per-draw slice); `offset` must satisfy
    // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT for a uniform-buffer target.
    void bind_buffer_range(BufferTarget target, uint32_t index, uint32_t buffer,
                           ptrdiff_t offset, ptrdiff_t size);

    void set_buffer_data(BufferTarget target, ptrdiff_t size,
                         const void* data, BufferUsage usage);

    inline void allocate_buffer(BufferTarget target,
                                ptrdiff_t size,
                                BufferUsage usage)
    {
        set_buffer_data(target, size, nullptr, usage);
    }

    inline void set_buffer_data(BufferTarget target,
                                std::span<const std::byte> data,
                                BufferUsage usage)
    {
        set_buffer_data(target, ptrdiff_t(data.size()), data.data(), usage);
    }

    template <typename T>
    void set_buffer_data(BufferTarget target,
                         std::span<T> data,
                         BufferUsage usage)
    {
        set_buffer_data(target, std::as_bytes(data), usage);
    }

    void set_buffer_subdata(BufferTarget target, ptrdiff_t offset,
                            ptrdiff_t size, const void* data);

    inline void set_buffer_subdata(BufferTarget target,
                                   std::span<const std::byte> data,
                                   ptrdiff_t offset = 0)
    {
        set_buffer_subdata(target, offset, ptrdiff_t(data.size()), data.data());
    }

    template <typename T>
    void set_buffer_subdata(BufferTarget target,
                            std::span<T> data,
                            ptrdiff_t offset = 0)
    {
        set_buffer_subdata(target, std::as_bytes(data), offset);
    }

    void resize_buffer(uint32_t buffer_id, ptrdiff_t new_size);

    BufferHandle clone_buffer(uint32_t buffer_id,
                              ptrdiff_t size = PTRDIFF_MAX);

    void copy_buffer(BufferTarget read_target,
                     ptrdiff_t read_offset,
                     BufferTarget write_target,
                     ptrdiff_t write_offset,
                     ptrdiff_t size);

    [[nodiscard]]
    bool is_buffer(uint32_t buffer);

    [[nodiscard]]
    ptrdiff_t get_buffer_size(BufferTarget target);

    [[nodiscard]]
    BufferUsage get_buffer_usage(BufferTarget target);

    [[nodiscard]]
    uint32_t get_bound_buffer(BufferTarget target);

    class BufferRestorer
    {
    public:
        explicit BufferRestorer(BufferTarget target);

        BufferRestorer(BufferTarget target, uint32_t previous_buffer_id);

        ~BufferRestorer();

    private:
        BufferTarget target_;
        uint32_t previous_buffer_id_;
    };
}
