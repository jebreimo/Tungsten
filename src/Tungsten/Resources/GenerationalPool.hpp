//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Resources/ResourceRef.hpp"

namespace Tungsten
{
    /**
     * The generational slot-table machinery shared by every owned resource type
     * (Mesh, Material, ShaderProgram, Texture, BufferArena) — the ArenaSlot
     * pattern generalized once. ResourceManager holds one instance per
     * type and forwards its create_* / get_* / destroy_* to insert / get / erase.
     *
     * A ref stays a revocable {index, generation} key: freeing a slot bumps
     * its generation, so a stale ref fails get() rather than aliasing a reused
     * slot. Generation 0 is reserved for the null ref, so live slots start at 1.
     */
    template <typename T>
    class GenerationalPool
    {
    public:
        /**
         * Moves a fully-built resource into a free slot (reused from the
         * free-list, or appended) and returns its ref.
         */
        ResourceRef<T> insert(T&& value)
        {
            uint32_t index;
            if (!free_list_.empty())
            {
                index = free_list_.back();
                free_list_.pop_back();
            }
            else
            {
                index = static_cast<uint32_t>(slots_.size());
                slots_.emplace_back();
            }
            slots_[index].value.emplace(std::move(value));
            return ResourceRef<T>{index, slots_[index].generation};
        }

        /**
         * Resolves a ref, throwing if it is out of range, stale (generation
         * mismatch), or points at a freed slot.
         */
        [[nodiscard]]
        T& get(ResourceRef<T> ref)
        {
            return *slots_[validate(ref)].value;
        }

        [[nodiscard]]
        const T& get(ResourceRef<T> ref) const
        {
            return *slots_[validate(ref)].value;
        }

        /**
         * Frees the slot: hands the departing value to on_retire (so the caller
         * can move its GL handles onto the DeletionQueue), then empties the
         * slot, bumps its generation to revoke outstanding refs, and returns the
         * index to the free-list. on_retire is called *before* the value is
         * destroyed, so anything it does not move out is deleted inline by the
         * reset that follows — the caller must move out any GL-owning members it
         * wants deferred.
         */
        template <typename OnRetire>
        void erase(ResourceRef<T> ref, OnRetire&& on_retire)
        {
            const uint32_t index = validate(ref);
            Slot& slot = slots_[index];
            on_retire(std::move(*slot.value));
            slot.value.reset();
            ++slot.generation;
            free_list_.push_back(index);
        }

    private:
        struct Slot
        {
            std::optional<T> value;   // empty marks a free slot
            uint32_t generation = 1;  // 0 is reserved for the null ref
        };

        [[nodiscard]]
        uint32_t validate(ResourceRef<T> ref) const
        {
            if (ref.index >= slots_.size())
                TUNGSTEN_THROW("GenerationalPool: ref index out of range.");
            const Slot& slot = slots_[ref.index];
            if (!slot.value || slot.generation != ref.generation)
                TUNGSTEN_THROW("GenerationalPool: stale ref.");
            return ref.index;
        }

        std::vector<Slot> slots_;
        std::vector<uint32_t> free_list_;
    };
} // Tungsten
