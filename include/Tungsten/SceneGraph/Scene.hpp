//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <concepts>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>
#include <Xyz/Matrix.hpp>
#include "Tungsten/TungstenException.hpp"
#include "CameraComponent.hpp"
#include "LightComponent.hpp"
#include "NodeId.hpp"
#include "RenderableComponent.hpp"
#include "TextComponent.hpp"
#include "Transform.hpp"

namespace Tungsten
{
    class NodeHandle;

    // The component kinds the scene stores. Components are not polymorphic:
    // each kind gets its own flat array, so the set is closed and adding a
    // kind means adding an array. In exchange the per-frame passes iterate one
    // contiguous array of a known type, with no virtual calls and no casts.
    template <typename T>
    concept ComponentType = std::same_as<T, RenderableComponent>
                            || std::same_as<T, LightComponent>
                            || std::same_as<T, CameraComponent>
                            || std::same_as<T, TextComponent>;

    // One kind's components and the nodes they belong to, in lockstep:
    // owners[i] is the node that items[i] is attached to. Neither order is
    // meaningful — removal is a swap-and-pop.
    template <ComponentType T>
    struct ComponentStore
    {
        std::vector<T> items;
        std::vector<NodeId> owners;
    };

    /**
     * The mutable scene graph. Nodes are not objects: the scene owns one
     * flat array per node attribute, all indexed by NodeId::index, and a node
     * is just that index. Nothing is allocated per node and nothing holds a
     * pointer to one, so the per-frame passes are loops over contiguous
     * memory rather than pointer chases through a tree.
     *
     * The hierarchy is stored as first-child / next-sibling links rather than
     * a child vector per node, which keeps structural edits O(1) and keeps the
     * topology in the same flat arrays as everything else. Roots are linked
     * through the same sibling chain, so they need no special case.
     */
    class Scene
    {
    public:
        /**
         * Iterates a node's children through the sibling chain. Yields NodeIds
         * and allocates nothing.
         */
        class ChildIterator
        {
        public:
            ChildIterator() = default;

            ChildIterator(const Scene* scene, NodeId id)
                : scene_(scene), id_(id)
            {}

            NodeId operator*() const
            {
                return id_;
            }

            ChildIterator& operator++()
            {
                id_ = scene_->next_sibling(id_);
                return *this;
            }

            bool operator==(const ChildIterator& other) const
            {
                return id_ == other.id_;
            }

        private:
            const Scene* scene_ = nullptr;
            NodeId id_;
        };

        /**
         * A range of a node's children, iterable with a range-based for loop.
         */
        class ChildRange
        {
        public:
            ChildRange(const Scene* scene, NodeId first)
                : scene_(scene), first_(first)
            {}

            [[nodiscard]]
            ChildIterator begin() const
            {
                return {scene_, first_};
            }

            [[nodiscard]]
            ChildIterator end() const
            {
                return {scene_, NodeId{}};
            }

            [[nodiscard]]
            bool empty() const
            {
                return !first_;
            }

        private:
            const Scene* scene_;
            NodeId first_;
        };

        /**
         * Creates a node and returns its id. A null parent makes it a root;
         * otherwise it is appended after the parent's existing children.
         * Throws if parent is non-null and does not name a live node.
         */
        NodeId create_node(NodeId parent = {});

        /**
         * Creates a node and returns a handle to it — the ergonomic spelling
         * of create_node().
         */
        NodeHandle add_node(NodeId parent = {});

        /**
         * Destroys a node and its whole subtree, along with every component
         * attached to any of them. The removed nodes' generations are bumped,
         * so ids naming them stop resolving. Throws if id does not name a live
         * node.
         */
        void remove(NodeId id);

        /**
         * Moves a node (with its subtree) under new_parent, or makes it a root
         * if new_parent is null. Throws if either id is invalid, or if
         * new_parent is inside the node's own subtree, which would make a
         * cycle.
         */
        void reparent(NodeId id, NodeId new_parent);

        /**
         * Returns whether a node is alive.
         */
        [[nodiscard]]
        bool is_alive(NodeId id) const;

        [[nodiscard]]
        NodeId parent(NodeId id) const;

        [[nodiscard]]
        NodeId next_sibling(NodeId id) const;

        [[nodiscard]]
        ChildRange children(NodeId id) const;

        [[nodiscard]]
        const Transform& local_transform(NodeId id) const;

        void set_local_transform(NodeId id, const Transform& transform);

        /**
         * Returns the node's world transform as of the last resolve_transforms().
         * Unlike the old lazy scheme this does not recompute on access — the
         * linear pass is cheap enough that resolving on demand would cost more
         * bookkeeping than it saves.
         */
        [[nodiscard]]
        const Xyz::Matrix4F& world_matrix(NodeId id) const;

        /**
         * Recomputes every live node's world matrix in one pass over `order_`,
         * rebuilding that order first if the hierarchy changed.
         */
        void resolve_transforms();

        /**
         * Attaches a component to a node and returns a reference to it inside
         * the store. That reference is invalidated by the next add_component
         * of the same kind — hold the NodeId, not the pointer. Throws if id
         * does not name a live node.
         *
         * A node may carry more than one component of a kind; find/get return
         * the first.
         */
        template <ComponentType T>
        T& add_component(NodeId id, T component)
        {
            std::ignore = validate(id);
            auto& store = component_store(static_cast<T*>(nullptr));
            store.items.push_back(std::move(component));
            store.owners.push_back(id);
            return store.items.back();
        }

        template <ComponentType T>
        [[nodiscard]]
        T* find_component(NodeId id)
        {
            auto& store = component_store(static_cast<T*>(nullptr));
            for (size_t i = 0; i < store.owners.size(); ++i)
            {
                if (store.owners[i] == id)
                    return &store.items[i];
            }
            return nullptr;
        }

        template <ComponentType T>
        [[nodiscard]]
        const T* find_component(NodeId id) const
        {
            return const_cast<Scene*>(this)->find_component<T>(id);
        }

        /**
         * Returns the node's component of this kind. Throws if it has none.
         */
        template <ComponentType T>
        [[nodiscard]]
        T& get_component(NodeId id)
        {
            if (T* component = find_component<T>(id))
                return *component;
            TUNGSTEN_THROW("Scene: the node has no component of that kind.");
        }

        template <ComponentType T>
        [[nodiscard]]
        const T& get_component(NodeId id) const
        {
            return const_cast<Scene*>(this)->get_component<T>(id);
        }

        /**
         * Detaches the node's first component of this kind. Does nothing if it
         * has none.
         */
        template <ComponentType T>
        void remove_component(NodeId id)
        {
            auto& store = component_store(static_cast<T*>(nullptr));
            for (size_t i = 0; i < store.owners.size(); ++i)
            {
                if (store.owners[i] != id)
                    continue;
                store.items[i] = std::move(store.items.back());
                store.owners[i] = store.owners.back();
                store.items.pop_back();
                store.owners.pop_back();
                return;
            }
        }

        /**
         * Returns every component of one kind, for the passes that sweep them
         * all.
         */
        template <ComponentType T>
        [[nodiscard]]
        const ComponentStore<T>& components() const
        {
            return const_cast<Scene*>(this)
                ->component_store(static_cast<T*>(nullptr));
        }

        /**
         * The same store, writable, for the systems that sweep a kind and put
         * derived state back into it — TextSystem, which compiles every
         * TextComponent into a mesh and records what it built in the component.
         * Extraction passes take the const overload instead.
         */
        template <ComponentType T>
        [[nodiscard]]
        ComponentStore<T>& components()
        {
            return component_store(static_cast<T*>(nullptr));
        }

    private:
        /**
         * Unlinks id from its parent's (or the root) sibling chain.
         */
        void unlink(NodeId id);

        /**
         * Appends id to the end of parent's child chain, or the root chain if
         * parent is null.
         */
        void link(NodeId id, NodeId parent);

        /**
         * Rebuilds `order_` so that every parent precedes its children. The
         * output vector doubles as the queue: appending each node's children
         * as the walk passes over it visits the graph breadth-first, which
         * gives the invariant for free and keeps siblings in insertion order.
         */
        void rebuild_order();

        /**
         * Throws unless id names a live node; returns its index, which most
         * callers want and the rest ignore.
         */
        uint32_t validate(NodeId id) const; // NOLINT(*-use-nodiscard)

        /**
         * Picks the store for a component kind. Tag dispatch on a null T*
         * rather than an if-constexpr chain, so an unsupported kind fails with
         * "no matching function" instead of a template error inside Scene.
         */
        ComponentStore<RenderableComponent>& component_store(
            RenderableComponent*)
        {
            return renderables_;
        }

        ComponentStore<LightComponent>& component_store(LightComponent*)
        {
            return lights_;
        }

        ComponentStore<CameraComponent>& component_store(CameraComponent*)
        {
            return cameras_;
        }

        ComponentStore<TextComponent>& component_store(TextComponent*)
        {
            return texts_;
        }

        /**
         * Drops every component whose owning node is gone. Called after a
         * removal, which has already bumped the doomed nodes' generations.
         */
        template <ComponentType T>
        void drop_orphaned_components(ComponentStore<T>& store)
        {
            size_t out = 0;
            for (size_t i = 0; i < store.owners.size(); ++i)
            {
                if (!is_alive(store.owners[i]))
                    continue;
                if (out != i)
                {
                    store.items[out] = std::move(store.items[i]);
                    store.owners[out] = store.owners[i];
                }
                ++out;
            }
            store.items.resize(out);
            store.owners.resize(out);
        }

        /**
         * Arrays indexed by NodeId::index. Slots are reused through
         * free_list_, and generations_ revokes ids to the previous occupant.
         *
         * This repeats the slot-table bookkeeping GenerationalPool already
         * implements, and deliberately so: that class stores one
         * std::optional<T> per slot, so every node's fields would sit together
         * in one struct. The point of this class is the opposite — one
         * contiguous array per field, so a pass that only reads parents and
         * locals touches only those two. Folding the two together would undo
         * that. Keep them separate.
         */
        std::vector<uint32_t> generations_;
        std::vector<NodeId> parents_;
        std::vector<NodeId> first_children_;
        std::vector<NodeId> next_siblings_;
        std::vector<Transform> locals_;
        std::vector<Xyz::Matrix4F> worlds_;
        std::vector<uint8_t> alive_;
        std::vector<uint32_t> free_list_;

        NodeId first_root_;
        /**
         * Every live node, with parents before children. Rebuilt only when the
         * hierarchy changes. This is what lets resolve_transforms() be a
         * single linear pass.
         */
        std::vector<uint32_t> order_;
        bool hierarchy_dirty_ = false;

        ComponentStore<RenderableComponent> renderables_;
        ComponentStore<LightComponent> lights_;
        ComponentStore<CameraComponent> cameras_;
        ComponentStore<TextComponent> texts_;
    };
} // Tungsten
