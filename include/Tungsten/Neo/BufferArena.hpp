//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <optional>
#include <utility>
#include "Tungsten/Detail/BuddyAllocator.hpp"
#include "Tungsten/Gl/GlBuffer.hpp"

namespace Tungsten
{
    // All offsets, counts and capacities are in stride units (vertices or
    // indices), not bytes: an offset is directly the base vertex/index a draw
    // uses. Byte offsets only exist where a GL call needs one, computed via
    // stride().
    class BufferArena
    {
    public:
        BufferArena(BufferUsage usage, uint16_t stride, uint32_t capacity);

        // Returns the unit offset of a free range of `count` units, or nullopt
        // if the arena is full. Never grows the buffer; the owner decides when
        // and how much to grow (see ResourceManager::allocate).
        [[nodiscard]]
        std::optional<uint32_t> allocate(uint32_t count);

        void free(uint32_t offset);

        // Grows the backing buffer to hold at least new_capacity units,
        // preserving every live allocation at its current offset, and returns
        // the displaced (old) GL buffer for the caller to retire. The buffer id
        // changes, so VAOs referencing this arena must be rebuilt afterwards.
        // Throws if new_capacity is not larger than the current capacity.
        [[nodiscard]]
        BufferHandle grow(uint32_t new_capacity);

        // Moves the GL buffer out, for retiring it when the arena itself is
        // destroyed. The arena must not be used afterwards.
        [[nodiscard]]
        BufferHandle release_buffer()
        {
            return std::move(buffer_);
        }

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

        [[nodiscard]]
        uint32_t capacity() const
        {
            return static_cast<uint32_t>(allocator_.capacity());
        }

        [[nodiscard]]
        bool empty() const
        {
            return allocator_.allocated() == 0;
        }
    private:
        BufferHandle buffer_;
        BufferUsage usage_;
        uint16_t stride_;
        BuddyAllocator allocator_;
    };
} // Tungsten
