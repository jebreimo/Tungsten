//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../src/Tungsten/Detail/BuddyAllocator.hpp"

#include <catch2/catch_test_macros.hpp>

using Tungsten::BuddyAllocator;

TEST_CASE("BuddyAllocator: capacity rounds up to power of two")
{
    REQUIRE(BuddyAllocator(7).capacity() == 8);
    REQUIRE(BuddyAllocator(8).capacity() == 8);
    REQUIRE(BuddyAllocator(9).capacity() == 16);
    REQUIRE(BuddyAllocator(1024).capacity() == 1024);
}

TEST_CASE("BuddyAllocator: allocate and free the full capacity")
{
    BuddyAllocator a(8);
    auto off = a.allocate(8);
    REQUIRE(off == 0);
    REQUIRE(a.allocated() == 8);
    a.free(*off);
    REQUIRE(a.allocated() == 0);
    // Should be allocatable again
    REQUIRE(a.allocate(8) == 0);
}

TEST_CASE("BuddyAllocator: size is rounded up to next power of two")
{
    BuddyAllocator a(16);
    auto off = a.allocate(3); // allocated as size 4
    REQUIRE(off.has_value());
    REQUIRE(a.allocated() == 4);
    a.free(*off);
    REQUIRE(a.allocated() == 0);
}

TEST_CASE("BuddyAllocator: two equal allocations split the buffer")
{
    BuddyAllocator a(8);
    auto a0 = a.allocate(4);
    auto a1 = a.allocate(4);
    REQUIRE(a0.has_value());
    REQUIRE(a1.has_value());
    REQUIRE(*a0 != *a1);
    REQUIRE(a.allocated() == 8);
    REQUIRE_FALSE(a.allocate(1).has_value()); // full
    a.free(*a0);
    REQUIRE(a.allocated() == 4);
    a.free(*a1);
    REQUIRE(a.allocated() == 0);
}

TEST_CASE("BuddyAllocator: freed buddies coalesce")
{
    BuddyAllocator a(8);
    auto a0 = a.allocate(4);
    auto a1 = a.allocate(4);
    REQUIRE(a0.has_value());
    REQUIRE(a1.has_value());
    a.free(*a0);
    a.free(*a1);
    // Both halves freed; should be able to allocate the full 8 again
    auto full = a.allocate(8);
    REQUIRE(full.has_value());
    REQUIRE(*full == 0);
}

TEST_CASE("BuddyAllocator: many small allocations")
{
    BuddyAllocator a(16);
    // 16 allocations of size 1
    std::vector<size_t> offsets;
    for (int i = 0; i < 16; ++i)
    {
        auto off = a.allocate(1);
        REQUIRE(off.has_value());
        offsets.push_back(*off);
    }
    REQUIRE(a.allocated() == 16);
    REQUIRE_FALSE(a.allocate(1).has_value());

    // All offsets are distinct
    std::set<size_t> unique(offsets.begin(), offsets.end());
    REQUIRE(unique.size() == 16);

    // Free all and verify full coalescing
    for (size_t off : offsets)
        a.free(off);
    REQUIRE(a.allocated() == 0);
    REQUIRE(a.allocate(16).has_value());
}

TEST_CASE("BuddyAllocator: allocate zero or oversized returns nullopt")
{
    BuddyAllocator a(8);
    REQUIRE_FALSE(a.allocate(0).has_value());
    REQUIRE_FALSE(a.allocate(9).has_value());
}

TEST_CASE("BuddyAllocator: claim reserves a specific block")
{
    // Allocate a normal block, then build a fresh allocator and claim the same slot.
    BuddyAllocator a(16);
    auto off = a.allocate(4);
    REQUIRE(off.has_value());
    const size_t claimed_offset = *off;

    // Rebuild: new allocator, claim the same block.
    BuddyAllocator b(16);
    REQUIRE(b.claim(claimed_offset, 4));
    REQUIRE(b.allocated() == 4);

    // The claimed slot must not be handed out by a subsequent allocate.
    for (int i = 0; i < 12; ++i) // allocate the remaining 12 units
        (void)b.allocate(1);
    // Buffer should now be full — claimed slot was never re-used.
    REQUIRE_FALSE(b.allocate(1).has_value());
}

TEST_CASE("BuddyAllocator: claim after doubling rebuilds correctly")
{
    // Simulate growing: old capacity 8, two allocations at offsets 0 and 4.
    BuddyAllocator old_alloc(8);
    auto off0 = old_alloc.allocate(4);
    auto off4 = old_alloc.allocate(4);
    REQUIRE(off0 == 0);
    REQUIRE(off4 == 4);

    // Simulate buffer growth: new allocator with double capacity.
    BuddyAllocator new_alloc(16);
    REQUIRE(new_alloc.claim(*off0, 4));
    REQUIRE(new_alloc.claim(*off4, 4));
    REQUIRE(new_alloc.allocated() == 8);

    // The old region [0,8) is fully claimed; [8,16) is free.
    auto extra = new_alloc.allocate(8);
    REQUIRE(extra.has_value());
    REQUIRE(*extra == 8); // must be in the upper half
}

TEST_CASE("BuddyAllocator: claim on an already-allocated offset returns false")
{
    BuddyAllocator a(8);
    REQUIRE(a.claim(0, 4));
    REQUIRE_FALSE(a.claim(0, 4)); // same block, already taken
    REQUIRE_FALSE(a.claim(2, 2)); // overlapping block, also taken
}

TEST_CASE("BuddyAllocator: interleaved alloc/free coalesces correctly")
{
    BuddyAllocator a(8);
    auto a0 = a.allocate(2); // [0, 2)
    auto a1 = a.allocate(2); // [2, 4)
    auto a2 = a.allocate(2); // [4, 6)
    auto a3 = a.allocate(2); // [6, 8)
    REQUIRE(a0.has_value());
    REQUIRE(a1.has_value());
    REQUIRE(a2.has_value());
    REQUIRE(a3.has_value());

    a.free(*a1); // free [2, 4)
    a.free(*a0); // free [0, 2) — should coalesce with [2, 4) into [0, 4)
    // Now [0, 4) is free, [4, 8) is allocated
    REQUIRE_FALSE(a.allocate(8).has_value()); // still can't get full block
    auto big = a.allocate(4);
    REQUIRE(big.has_value());
    REQUIRE(*big == 0);
}