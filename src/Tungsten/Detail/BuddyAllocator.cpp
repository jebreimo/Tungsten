//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BuddyAllocator.hpp"

#include <bit>
#include <cassert>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    BuddyAllocator::BuddyAllocator(size_t capacity)
        : capacity_(std::bit_ceil(capacity)),
          max_level_(int(std::bit_width(capacity_)) - 1),
          free_lists_(max_level_ + 1)
    {
        free_lists_[max_level_].insert(0);
    }

    std::optional<size_t> BuddyAllocator::allocate(size_t size)
    {
        if (size == 0 || size > capacity_)
            return std::nullopt;

        const int target_level = level_of(size);

        // Find the smallest free block that fits
        int found_level = -1;
        for (int l = target_level; l <= max_level_; ++l)
        {
            if (!free_lists_[l].empty())
            {
                found_level = l;
                break;
            }
        }
        if (found_level == -1)
            return std::nullopt;

        auto it = free_lists_[found_level].begin();
        size_t offset = *it;
        free_lists_[found_level].erase(it);

        // Split down to target level, freeing the upper half at each step
        while (found_level > target_level)
        {
            --found_level;
            free_lists_[found_level].insert(offset + block_size(found_level));
        }

        allocations_[offset] = target_level;
        allocated_ += block_size(target_level);
        return offset;
    }

    void BuddyAllocator::free(size_t offset)
    {
        auto it = allocations_.find(offset);
        assert(it != allocations_.end());
        int level = it->second;
        allocations_.erase(it);
        allocated_ -= block_size(level);

        // Merge with buddy as long as the buddy is free
        while (level < max_level_)
        {
            const size_t buddy = offset ^ block_size(level);
            const auto buddy_it = free_lists_[level].find(buddy);
            if (buddy_it == free_lists_[level].end())
                break;
            free_lists_[level].erase(buddy_it);
            offset = std::min(offset, buddy);
            ++level;
        }

        free_lists_[level].insert(offset);
    }

    bool BuddyAllocator::claim(size_t offset, size_t size)
    {
        if (size == 0 || size > capacity_)
            return false;

        const int target_level = level_of(size);
        if (offset % block_size(target_level) != 0 || offset + block_size(target_level) > capacity_)
            return false;

        // Walk down from max_level to find the free ancestor block that
        // contains offset, then split it down to target_level.
        for (int l = max_level_; l >= target_level; --l)
        {
            const size_t bs = block_size(l);
            const size_t block_start = (offset / bs) * bs;

            auto it = free_lists_[l].find(block_start);
            if (it == free_lists_[l].end())
                continue;

            free_lists_[l].erase(it);

            size_t current = block_start;
            while (l > target_level)
            {
                --l;
                const size_t upper_half = current + block_size(l);
                if (offset >= upper_half)
                {
                    free_lists_[l].insert(current);
                    current = upper_half;
                }
                else
                {
                    free_lists_[l].insert(upper_half);
                }
            }

            allocations_[current] = target_level;
            allocated_ += block_size(target_level);
            return true;
        }

        return false;
    }

    BuddyAllocator BuddyAllocator::resized(size_t new_capacity) const
    {
        BuddyAllocator result(new_capacity);
        for (const auto& [offset, level] : allocations_)
        {
            const size_t size = block_size(level);
            if (offset + size > result.capacity_)
            {
                TUNGSTEN_THROW("BuddyAllocator: new capacity is too small to"
                               " preserve an existing allocation.");
            }
            const bool claimed = result.claim(offset, size);
            assert(claimed);
            (void)claimed;
        }
        return result;
    }

    size_t BuddyAllocator::capacity() const
    {
        return capacity_;
    }

    size_t BuddyAllocator::allocated() const
    {
        return allocated_;
    }

    int BuddyAllocator::level_of(size_t size) const
    {
        return int(std::bit_width(std::bit_ceil(size))) - 1;
    }

    size_t BuddyAllocator::block_size(int level) const
    {
        return size_t(1) << level;
    }
}
