//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BufferArena.hpp"

#include <bit>
#include <limits>
#include <utility>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    BufferArena::BufferArena(BufferUsage usage, uint16_t stride, uint32_t capacity)
        : usage_(usage),
          stride_(stride),
          capacity_(capacity),
          allocator_(capacity)
    {
        // A zero-capacity arena creates no buffer until it first grows.
        if (capacity_ != 0)
        {
            buffer_ = generate_buffer(
                static_cast<ptrdiff_t>(capacity_) * stride_, usage_);
        }
    }

    std::optional<uint32_t> BufferArena::allocate(uint32_t count)
    {
        const auto offset = allocator_.allocate(count);
        if (!offset)
            return std::nullopt;

        // The allocator's range may extend past the end of the buffer.
        if (*offset + count > capacity_)
        {
            allocator_.free(*offset);
            return std::nullopt;
        }

        return static_cast<uint32_t>(*offset);
    }

    void BufferArena::free(uint32_t offset)
    {
        allocator_.free(offset);
    }

    BufferHandle BufferArena::grow(uint32_t new_capacity)
    {
        const auto new_capacity64 = std::bit_ceil(static_cast<uint64_t>(new_capacity));
        if (new_capacity64 <= capacity_)
            TUNGSTEN_THROW("BufferArena: new capacity must be greater than current capacity");
        if (new_capacity64 > std::numeric_limits<uint32_t>::max())
            TUNGSTEN_THROW("BufferArena: capacity overflow");

        const auto new_size = static_cast<ptrdiff_t>(new_capacity64 * stride_);
        auto newBuffer = buffer_
                             ? clone_buffer(buffer_.id(), new_size)
                             : generate_buffer(new_size, usage_);
        // The allocator may already cover the new capacity if the old one
        // was not a power of two.
        if (new_capacity64 > allocator_.capacity())
            allocator_ = allocator_.resized(new_capacity64);
        capacity_ = static_cast<uint32_t>(new_capacity64);
        std::swap(buffer_, newBuffer);
        return newBuffer;
    }

    BufferHandle BufferArena::reset(uint32_t new_capacity)
    {
        if (!empty())
            TUNGSTEN_THROW("BufferArena: only an empty arena can be reset");

        BufferHandle newBuffer;
        if (new_capacity != 0)
        {
            newBuffer = generate_buffer(
                static_cast<ptrdiff_t>(new_capacity) * stride_, usage_);
        }
        allocator_ = BuddyAllocator(new_capacity);
        capacity_ = new_capacity;
        std::swap(buffer_, newBuffer);
        return newBuffer;
    }
} // Tungsten
