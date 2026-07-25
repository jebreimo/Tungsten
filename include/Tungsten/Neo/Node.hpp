//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <Xyz/BBox.hpp>
#include <Xyz/Matrix.hpp>
#include "Component.hpp"
#include "Transform.hpp"

namespace Tungsten
{
    // A node in the scene graph (§2): a local transform, the owned list of
    // children, and the components that give the node meaning.
    //
    // Children and components are owned through unique_ptr so that the raw
    // back-pointers below them (Node::parent, Component::owner) stay valid
    // when sibling lists grow; a node's address is stable for its lifetime,
    // which is also why nodes can be neither copied nor moved.
    //
    // The world transform is resolved lazily and only where something changed
    // (§2): world_matrix() recomputes iff the local transform changed or the
    // parent's worldVersion differs from the version this node last saw.
    // Nothing walks the whole tree per frame.
    class Node
    {
    public:
        Node() = default;

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        [[nodiscard]]
        const Transform& local_transform() const;

        // The only way to change the transform, so the dirty flag cannot be
        // bypassed (§2).
        void set_local_transform(const Transform& transform);

        [[nodiscard]]
        Node* parent() const;

        [[nodiscard]]
        const std::vector<std::unique_ptr<Node>>& children() const;

        // Attaches child and returns a reference to it. Throws if child is
        // null, already has a parent, or is an ancestor of this node (which
        // would create a cycle). The child is marked dirty, so its world
        // matrix is recomputed on next access.
        Node& add_child(std::unique_ptr<Node> child);

        // Detaches child and returns ownership of it, so the caller can
        // attach it elsewhere or drop it. Throws if child is not a child of
        // this node. The detached node is marked dirty: with no parent left
        // to compare versions against, its next world_matrix() recomputes as
        // a root.
        std::unique_ptr<Node> remove_child(Node& child);

        // Moves this node from its current parent to new_parent. Throws if
        // the node has no parent — a root is owned by the Scene, so detach it
        // with Scene::remove and attach it with add_child instead.
        void reparent(Node& new_parent);

        // Takes ownership of component, sets its owner to this node, and
        // returns a reference to it.
        Component& add_component(std::unique_ptr<Component> component);

        [[nodiscard]]
        const std::vector<std::unique_ptr<Component>>& components() const;

        // The node's world transform, resolved lazily (§2). Resolving a node
        // resolves its ancestors first; unchanged subtrees cost one version
        // comparison.
        [[nodiscard]]
        const Xyz::Matrix4F& world_matrix() const;

        // Bumped every time the world matrix is recomputed. Children compare
        // it with the value they saw at their own last recompute to detect
        // that an ancestor moved.
        [[nodiscard]]
        uint64_t world_version() const;

        // The aggregate world-space bounds: this node's renderables unioned
        // with its children's world bounds (§2, "bounds propagate up").
        // Maintained by TransformUpdater::resolve; empty until the first
        // resolve, or when the subtree has no renderables with bounds.
        [[nodiscard]]
        const Xyz::BBox3F& world_bounds() const
        {
            return world_bounds_;
        }

    private:
        friend class TransformUpdater;

        Transform local_transform_;
        std::vector<std::unique_ptr<Node>> children_;
        std::vector<std::unique_ptr<Component>> components_;
        Node* parent_ = nullptr;

        // Derived caches (§2), updated through const traversals
        // (world_matrix() and TransformUpdater::resolve), hence mutable.
        mutable Xyz::BBox3F world_bounds_;
        mutable Xyz::Matrix4F world_matrix_;
        mutable uint64_t world_version_ = 0;
        mutable uint64_t parent_version_seen_ = 0;
        mutable bool local_dirty_ = true;
    };
} // Tungsten
