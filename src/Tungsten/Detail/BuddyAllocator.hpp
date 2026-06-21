//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace Tungsten
{
    // Manages sub-allocations within a fixed-size integer range [0, capacity).
    //
    // All blocks are power-of-two sized and naturally aligned to their size.
    // Allocations are rounded up to the next power of two, so internal
    // fragmentation is at most 2x. Freed blocks are immediately coalesced with
    // their buddy when possible.
    //
    // The capacity supplied to the constructor is rounded up to the next power
    // of two. Units are caller-defined (e.g. vertex count, byte count).
    class BuddyAllocator
    {
    public:
        explicit BuddyAllocator(size_t capacity);

        // Returns the starting offset of the allocated block, or nullopt when
        // no contiguous free block is large enough to satisfy the request.
        // The allocated block may be larger than size (rounded up to pow2).
        [[nodiscard]] std::optional<size_t> allocate(size_t size);

        // Releases the block at offset. The offset must be one previously
        // returned by allocate(); behaviour is undefined otherwise.
        void free(size_t offset);

        // Claims a specific block at offset with the given size (rounded up to
        // the next power of two). Used to reconstruct allocator state after
        // growing a buffer: create a new larger allocator and re-claim all the
        // blocks that were live in the old one. Returns false if the block
        // overlaps an already-claimed block or is out of range.
        bool claim(size_t offset, size_t size);

        // Total capacity (rounded up to the next power of two on construction).
        [[nodiscard]] size_t capacity() const;

        // Number of units currently in use (sum of all allocated block sizes).
        [[nodiscard]] size_t allocated() const;

    private:
        [[nodiscard]] int level_of(size_t size) const;
        [[nodiscard]] size_t block_size(int level) const;

        size_t capacity_;
        int max_level_;
        size_t allocated_ = 0;
        std::vector<std::set<size_t>> free_lists_;    // [level] -> free offsets
        std::unordered_map<size_t, int> allocations_; // offset -> level
    };
}