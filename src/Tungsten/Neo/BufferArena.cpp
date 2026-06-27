//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BufferArena.hpp"

#include <algorithm>
#include <bit>
#include <limits>
#include <utility>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    BufferArena::BufferArena(BufferUsage usage, uint16_t stride, uint32_t capacity)
        : usage_(usage),
          stride_(stride),
          allocator_(capacity)
    {
        buffer_ = generate_buffer(capacity * stride, usage);
    }

    BufferArena::Allocation BufferArena::allocate(uint32_t count)
    {
        if (const auto offset = allocator_.allocate(count))
            return {byte_offset(*offset), BufferHandle{}};

        const auto new_capacity = std::max(
            static_cast<uint64_t>(allocator_.capacity()) * 2,
            static_cast<uint64_t>(count) * (allocator_.allocated() ? 2 : 1));
        if (new_capacity > std::numeric_limits<uint32_t>::max())
            TUNGSTEN_THROW("BufferArena: capacity overflow.");

        BufferHandle retired = grow(static_cast<uint32_t>(new_capacity));
        const auto offset = allocator_.allocate(count);
        if (!offset)
            TUNGSTEN_THROW("BufferArena: allocation failed after grow.");
        return {byte_offset(*offset), std::move(retired)};
    }

    void BufferArena::free(uint32_t offset)
    {
        allocator_.free(offset / stride_);
    }

    BufferHandle BufferArena::grow(uint32_t new_capacity)
    {
        const auto new_capacity64 = std::bit_ceil(static_cast<uint64_t>(new_capacity));
        if (new_capacity64 <= allocator_.capacity())
            TUNGSTEN_THROW("BufferArena: new capacity must be greater than current capacity");
        if (new_capacity64 > std::numeric_limits<uint32_t>::max())
            TUNGSTEN_THROW("BufferArena: capacity overflow");

        auto newBuffer = clone_buffer(buffer_.id(), static_cast<ptrdiff_t>(new_capacity64 * stride_));
        allocator_ = allocator_.resized(new_capacity64);
        std::swap(buffer_, newBuffer);
        return newBuffer;
    }

    uint32_t BufferArena::byte_offset(size_t count_offset) const
    {
        return static_cast<uint32_t>(count_offset * stride_);
    }
} // Tungsten
