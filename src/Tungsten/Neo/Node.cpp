//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Node.hpp"

#include <utility>

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    const Transform& Node::local_transform() const
    {
        return local_transform_;
    }

    void Node::set_local_transform(const Transform& transform)
    {
        local_transform_ = transform;
        local_dirty_ = true;
    }

    Node* Node::parent() const
    {
        return parent_;
    }

    const std::vector<std::unique_ptr<Node>>& Node::children() const
    {
        return children_;
    }

    Node& Node::add_child(std::unique_ptr<Node> child)
    {
        if (!child)
            TUNGSTEN_THROW("Node: child is null.");
        if (child->parent_)
            TUNGSTEN_THROW("Node: child already has a parent.");
        for (const Node* node = this; node; node = node->parent_)
        {
            if (node == child.get())
                TUNGSTEN_THROW("Node: child is an ancestor of this node.");
        }

        child->parent_ = this;
        // Structural changes dirty the moved node rather than relying on the
        // version mismatch alone: worldVersion counters are per-node, so the
        // new parent's version could coincide with the parentVersionSeen
        // recorded under the old parent (§2).
        child->local_dirty_ = true;
        children_.push_back(std::move(child));
        return *children_.back();
    }

    std::unique_ptr<Node> Node::remove_child(Node& child)
    {
        for (auto it = children_.begin(); it != children_.end(); ++it)
        {
            if (it->get() != &child)
                continue;

            std::unique_ptr<Node> released = std::move(*it);
            children_.erase(it);
            released->parent_ = nullptr;
            // With no parent left to compare versions against, only the
            // dirty flag makes the next world_matrix() recompute (§2).
            released->local_dirty_ = true;
            return released;
        }
        TUNGSTEN_THROW("Node: not a child of this node.");
    }

    void Node::reparent(Node& new_parent)
    {
        if (!parent_)
            TUNGSTEN_THROW("Node: cannot reparent a node without a parent.");
        new_parent.add_child(parent_->remove_child(*this));
    }

    Component& Node::add_component(std::unique_ptr<Component> component)
    {
        if (!component)
            TUNGSTEN_THROW("Node: component is null.");
        if (component->owner_)
            TUNGSTEN_THROW("Node: component already has an owner.");

        component->owner_ = this;
        components_.push_back(std::move(component));
        return *components_.back();
    }

    const std::vector<std::unique_ptr<Component>>& Node::components() const
    {
        return components_;
    }

    const Xyz::Matrix4F& Node::world_matrix() const
    {
        if (parent_)
        {
            // Resolve the parent first: it may itself be stale, and its
            // version is only meaningful once it is up to date.
            const Xyz::Matrix4F& parent_world = parent_->world_matrix();
            if (local_dirty_ || parent_->world_version_ != parent_version_seen_)
            {
                world_matrix_ = parent_world * local_transform_.local_matrix();
                ++world_version_;
                parent_version_seen_ = parent_->world_version_;
                local_dirty_ = false;
            }
        }
        else if (local_dirty_)
        {
            world_matrix_ = local_transform_.local_matrix();
            ++world_version_;
            local_dirty_ = false;
        }
        return world_matrix_;
    }

    uint64_t Node::world_version() const
    {
        return world_version_;
    }
} // Tungsten
