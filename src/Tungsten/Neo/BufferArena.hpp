//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Detail/BuddyAllocator.hpp"
#include "Tungsten/Gl/GlBuffer.hpp"

namespace Tungsten
{
    class BufferArena
    {
    public:
        // The result of an allocation. If the allocation had to grow the
        // buffer, the displaced GL buffer is moved into retired_buffer so
        // the owner can defer-delete it; otherwise retired_buffer is empty.
        struct Allocation
        {
            uint32_t offset; // byte offset into the buffer
            BufferHandle retired_buffer;
        };

        BufferArena(BufferUsage usage, uint16_t stride, uint32_t capacity);

        [[nodiscard]]
        Allocation allocate(uint32_t count);

        void free(uint32_t offset);

        // Grows the backing buffer to hold at least new_capacity units,
        // preserving every live allocation at its current offset, and returns
        // the displaced (old) GL buffer for the caller to retire. The buffer id
        // changes, so VAOs referencing this arena must be rebuilt afterwards.
        // Throws if new_capacity is not larger than the current capacity.
        [[nodiscard]]
        BufferHandle grow(uint32_t new_capacity);

        [[nodiscard]]
        uint32_t buffer_id() const
        {
            return buffer_.id();
        }

        [[nodiscard]]
        BufferUsage usage() const
        {
            return usage_;
        }

        [[nodiscard]]
        uint16_t stride() const
        {
            return stride_;
        }
    private:
        // Converts a unit/count offset from the allocator into a byte offset
        // into the buffer.
        [[nodiscard]]
        uint32_t byte_offset(size_t count_offset) const;

        BufferHandle buffer_;
        BufferUsage usage_;
        uint16_t stride_;
        BuddyAllocator allocator_;
    };
} // Tungsten
