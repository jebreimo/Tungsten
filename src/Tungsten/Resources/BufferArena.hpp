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
    /**
     * A buffer arena is a buffer with a buddy allocator that manages free and
     * allocated regions within the buffer.
     *
     * The buffer arena does not grow automatically; the owner is responsible
     * for growing the buffer when needed. All offsets are in stride units
     * (vertices or indices), not bytes. Byte offsets only exist where a GL
     * call needs one, computed via stride().
     *
     * The GL buffer holds exactly the capacity it was created or reserved with,
     * which may be smaller than the BuddyAllocator's power-of-two range. An
     * allocation the allocator places past the end of the buffer is refused
     * as if the arena were full. Since an empty arena always allocates at
     * offset 0, a single allocation can use the whole buffer with no rounding
     * waste. Growing a non-empty arena rounds the capacity up to a power of
     * two.
     *
     * An arena with capacity 0 has no GL buffer (buffer_id() is 0) until
     * reserve() gives it one.
     */
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

        /**
         * Ensures the arena can hold at least new_capacity units, like
         * std::vector::reserve; a new_capacity not larger than the current
         * capacity is ignored. An empty arena gets a buffer of exactly
         * new_capacity units. A non-empty one is rounded up to a power of
         * two, and every live allocation keeps its offset and contents.
         *
         * Returns the displaced GL buffer for the caller to retire; it is
         * empty if nothing was displaced. Whenever the capacity changes, so
         * does buffer_id(), and VAOs referencing this arena must be rebuilt.
         */
        [[nodiscard]]
        BufferHandle reserve(uint32_t new_capacity);

        /**
         * Moves the GL buffer out, for retiring it when the arena itself is
         * destroyed. The arena must not be used afterwards.
         */
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
            return capacity_;
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
        uint32_t capacity_;
        BuddyAllocator allocator_;
    };
} // Tungsten
