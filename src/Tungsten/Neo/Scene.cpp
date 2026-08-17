//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/Scene.hpp"

#include "Tungsten/Neo/NodeHandle.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    NodeId Scene::create_node(NodeId parent)
    {
        if (parent)
            validate(parent);

        uint32_t index;
        if (!free_list_.empty())
        {
            index = free_list_.back();
            free_list_.pop_back();
        }
        else
        {
            index = static_cast<uint32_t>(generations_.size());
            generations_.push_back(1);
            parents_.emplace_back();
            first_children_.emplace_back();
            next_siblings_.emplace_back();
            locals_.emplace_back();
            worlds_.emplace_back();
            alive_.push_back(0);
        }

        locals_[index] = {};
        // A node read before the first resolve should look like an unmoved
        // node, not like uninitialized memory.
        worlds_[index] = locals_[index].make_matrix();
        first_children_[index] = {};
        alive_[index] = 1;

        const NodeId id{index, generations_[index]};
        link(id, parent);
        hierarchy_dirty_ = true;
        return id;
    }

    NodeHandle Scene::add_node(NodeId parent)
    {
        return {this, create_node(parent)};
    }

    void Scene::remove(NodeId id)
    {
        validate(id);
        unlink(id);

        // Collect the subtree breadth-first, using the output as the queue —
        // the same trick as rebuild_order().
        std::vector<uint32_t> doomed;
        doomed.push_back(id.index);
        for (size_t i = 0; i < doomed.size(); ++i)
        {
            for (NodeId child = first_children_[doomed[i]];
                 child;
                 child = next_siblings_[child.index])
            {
                doomed.push_back(child.index);
            }
        }

        for (const uint32_t index : doomed)
        {
            alive_[index] = 0;
            // Revokes every outstanding id to this node before the slot is
            // handed to the next one (§6).
            ++generations_[index];
            parents_[index] = {};
            first_children_[index] = {};
            next_siblings_[index] = {};
            free_list_.push_back(index);
        }

        drop_orphaned_components(renderables_);
        drop_orphaned_components(lights_);
        drop_orphaned_components(cameras_);

        hierarchy_dirty_ = true;
    }

    void Scene::reparent(NodeId id, NodeId new_parent)
    {
        validate(id);
        if (new_parent)
        {
            validate(new_parent);
            // Walking up from the new parent is enough: the move makes a cycle
            // exactly when the node is one of its own new ancestors.
            for (NodeId node = new_parent; node; node = parents_[node.index])
            {
                if (node == id)
                {
                    TUNGSTEN_THROW("Scene: cannot reparent a node into its"
                                   " own subtree.");
                }
            }
        }

        unlink(id);
        link(id, new_parent);
        hierarchy_dirty_ = true;
    }

    bool Scene::is_alive(NodeId id) const
    {
        return id
               && id.index < generations_.size()
               && alive_[id.index]
               && generations_[id.index] == id.generation;
    }

    NodeId Scene::parent(NodeId id) const
    {
        return parents_[validate(id)];
    }

    NodeId Scene::next_sibling(NodeId id) const
    {
        return next_siblings_[validate(id)];
    }

    Scene::ChildRange Scene::children(NodeId id) const
    {
        return {this, first_children_[validate(id)]};
    }

    const Transform& Scene::local_transform(NodeId id) const
    {
        return locals_[validate(id)];
    }

    void Scene::set_local_transform(NodeId id, const Transform& transform)
    {
        locals_[validate(id)] = transform;
    }

    const Xyz::Matrix4F& Scene::world_matrix(NodeId id) const
    {
        return worlds_[validate(id)];
    }

    void Scene::resolve_transforms()
    {
        if (hierarchy_dirty_)
            rebuild_order();

        for (const uint32_t index : order_)
        {
            const NodeId parent = parents_[index];
            worlds_[index] = parent
                             ? worlds_[parent.index]
                               * locals_[index].make_matrix()
                             : locals_[index].make_matrix();
        }
    }

    DoubleBuffer<RenderSnapshot>& Scene::snapshots()
    {
        return snapshots_;
    }

    void Scene::unlink(NodeId id)
    {
        const NodeId parent = parents_[id.index];
        NodeId& head = parent ? first_children_[parent.index] : first_root_;

        if (head == id)
        {
            head = next_siblings_[id.index];
        }
        else
        {
            NodeId previous = head;
            while (previous && next_siblings_[previous.index] != id)
                previous = next_siblings_[previous.index];
            if (previous)
                next_siblings_[previous.index] = next_siblings_[id.index];
        }

        parents_[id.index] = {};
        next_siblings_[id.index] = {};
    }

    void Scene::link(NodeId id, NodeId parent)
    {
        parents_[id.index] = parent;
        next_siblings_[id.index] = {};

        NodeId& head = parent ? first_children_[parent.index] : first_root_;
        if (!head)
        {
            head = id;
            return;
        }

        // Append rather than prepend, so children() yields them in the order
        // they were added.
        NodeId last = head;
        while (next_siblings_[last.index])
            last = next_siblings_[last.index];
        next_siblings_[last.index] = id;
    }

    void Scene::rebuild_order()
    {
        order_.clear();
        for (NodeId root = first_root_; root; root = next_siblings_[root.index])
            order_.push_back(root.index);

        // order_ is both the output and the queue: by the time the walk
        // reaches a node its parent has already been appended, which is
        // exactly the parents-before-children invariant the transform pass
        // relies on.
        for (size_t i = 0; i < order_.size(); ++i)
        {
            for (NodeId child = first_children_[order_[i]];
                 child;
                 child = next_siblings_[child.index])
            {
                order_.push_back(child.index);
            }
        }

        hierarchy_dirty_ = false;
    }

    uint32_t Scene::validate(NodeId id) const
    {
        if (id.index >= generations_.size())
            TUNGSTEN_THROW("Scene: node id index out of range.");
        if (!alive_[id.index] || generations_[id.index] != id.generation)
            TUNGSTEN_THROW("Scene: stale node id.");
        return id.index;
    }

} // Tungsten