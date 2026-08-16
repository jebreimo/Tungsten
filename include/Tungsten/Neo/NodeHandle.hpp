//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Scene.hpp"

namespace Tungsten
{
    // A node's data lives in Scene's arrays, so there is no node object to
    // hold a reference to. This is the ergonomic stand-in: a copyable
    // {scene, id} pair that turns scene.set_local_transform(id, t) back into
    // node.set_local_transform(t).
    //
    // It is a value, not a reference — copy it freely, but it is only as valid
    // as the id inside it, so a handle to a removed node throws when used
    // rather than resolving to whichever node took over the slot. Store the
    // handle rather than a pointer to a component: the component arrays move
    // their contents as they grow.
    class NodeHandle
    {
    public:
        NodeHandle() = default;

        NodeHandle(Scene* scene, NodeId id)
            : scene_(scene), id_(id)
        {}

        [[nodiscard]]
        NodeId id() const
        {
            return id_;
        }

        [[nodiscard]]
        Scene* scene() const
        {
            return scene_;
        }

        // True if the handle names something — not that the node is still
        // alive. Use Scene::is_alive for that.
        explicit operator bool() const
        {
            return scene_ && bool(id_);
        }

        bool operator==(const NodeHandle&) const = default;

        [[nodiscard]]
        const Transform& local_transform() const
        {
            return scene_->local_transform(id_);
        }

        void set_local_transform(const Transform& transform) const
        {
            scene_->set_local_transform(id_, transform);
        }

        [[nodiscard]]
        const Xyz::Matrix4F& world_matrix() const
        {
            return scene_->world_matrix(id_);
        }

        [[nodiscard]]
        NodeHandle parent() const
        {
            return {scene_, scene_->parent(id_)};
        }

        [[nodiscard]]
        Scene::ChildRange children() const
        {
            return scene_->children(id_);
        }

        // Creates a child of this node and returns a handle to it.
        [[nodiscard]]
        NodeHandle add_child() const
        {
            return scene_->add_node(id_);
        }

        void reparent(NodeHandle new_parent) const
        {
            scene_->reparent(id_, new_parent.id_);
        }

        // Destroys this node and its subtree. Every handle to any of them,
        // including this one, stops resolving.
        void remove() const
        {
            scene_->remove(id_);
        }

        // Attaches a component. The returned reference is invalidated by the
        // next add of the same kind, so reach for it through get<T>() each
        // time rather than caching it.
        template <ComponentType T>
        T& add(T component) const
        {
            return scene_->add_component(id_, std::move(component));
        }

        template <ComponentType T>
        [[nodiscard]]
        T& get() const
        {
            return scene_->get_component<T>(id_);
        }

        template <ComponentType T>
        [[nodiscard]]
        T* find() const
        {
            return scene_->find_component<T>(id_);
        }

        template <ComponentType T>
        void remove_component() const
        {
            scene_->remove_component<T>(id_);
        }

    private:
        Scene* scene_ = nullptr;
        NodeId id_;
    };
} // Tungsten