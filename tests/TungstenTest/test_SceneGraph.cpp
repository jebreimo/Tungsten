//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../src/Tungsten/Neo/Scene.hpp"

#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;
using Catch::Matchers::WithinAbs;

namespace
{
    Transform translate(float x, float y, float z)
    {
        Transform transform;
        transform.translation = {x, y, z};
        return transform;
    }

    Xyz::Vector3F world_translation(const Node& node)
    {
        const auto& m = node.world_matrix();
        return {m[0, 3], m[1, 3], m[2, 3]};
    }

    struct TestComponent : Component
    {
        int value = 0;
    };
}

TEST_CASE("Transform: the default is the identity")
{
    const auto m = Transform().local_matrix();
    for (size_t row = 0; row < 4; ++row)
    {
        for (size_t col = 0; col < 4; ++col)
            REQUIRE(m[row, col] == (row == col ? 1.0f : 0.0f));
    }
}

TEST_CASE("Transform: translation and scale compose as T * R * S")
{
    Transform transform;
    transform.translation = {1, 2, 3};
    transform.scale = {2, 4, 8};

    const auto m = transform.local_matrix();
    REQUIRE(m[0, 3] == 1.0f);
    REQUIRE(m[1, 3] == 2.0f);
    REQUIRE(m[2, 3] == 3.0f);
    REQUIRE(m[0, 0] == 2.0f);
    REQUIRE(m[1, 1] == 4.0f);
    REQUIRE(m[2, 2] == 8.0f);
}

TEST_CASE("Transform: yaw rotates about the z axis")
{
    Transform transform;
    transform.rotation.yaw = Xyz::Constants<float>::PI / 2;

    // A quarter turn maps the x axis onto the y axis.
    const auto m = transform.local_matrix();
    REQUIRE_THAT((m[0, 0]), WithinAbs(0, 1e-6));
    REQUIRE_THAT((m[1, 0]), WithinAbs(1, 1e-6));
}

TEST_CASE("Node: a root's world matrix is its local matrix")
{
    Node node;
    node.set_local_transform(translate(1, 2, 3));
    REQUIRE(world_translation(node) == Xyz::Vector3F{1, 2, 3});
}

TEST_CASE("Node: the world matrix is cached until something changes")
{
    Node node;
    node.set_local_transform(translate(1, 0, 0));

    REQUIRE(node.world_version() == 0);
    (void)node.world_matrix();
    REQUIRE(node.world_version() == 1);
    (void)node.world_matrix();
    REQUIRE(node.world_version() == 1); // no recompute

    node.set_local_transform(translate(2, 0, 0));
    (void)node.world_matrix();
    REQUIRE(node.world_version() == 2);
}

TEST_CASE("Node: children inherit their parent's world transform")
{
    Node parent;
    parent.set_local_transform(translate(1, 0, 0));
    auto& child = parent.add_child(std::make_unique<Node>());
    child.set_local_transform(translate(2, 0, 0));

    REQUIRE(world_translation(child) == Xyz::Vector3F{3, 0, 0});
}

TEST_CASE("Node: moving a parent invalidates resolved descendants")
{
    Node root;
    auto& mid = root.add_child(std::make_unique<Node>());
    auto& leaf = mid.add_child(std::make_unique<Node>());
    mid.set_local_transform(translate(1, 0, 0));
    leaf.set_local_transform(translate(1, 0, 0));

    REQUIRE(world_translation(leaf) == Xyz::Vector3F{2, 0, 0});
    const auto version = leaf.world_version();

    // An unchanged tree does not recompute.
    (void)leaf.world_matrix();
    REQUIRE(leaf.world_version() == version);

    root.set_local_transform(translate(10, 0, 0));
    REQUIRE(world_translation(leaf) == Xyz::Vector3F{12, 0, 0});
    REQUIRE(leaf.world_version() > version);
}

TEST_CASE("Node: reparenting picks up the new parent's transform")
{
    Node root;
    auto& a = root.add_child(std::make_unique<Node>());
    auto& b = root.add_child(std::make_unique<Node>());
    a.set_local_transform(translate(1, 0, 0));
    b.set_local_transform(translate(5, 0, 0));
    auto& child = a.add_child(std::make_unique<Node>());
    child.set_local_transform(translate(2, 0, 0));

    // Resolve everything, leaving a and b with *equal* world versions — the
    // case a bare parent-version comparison would miss on reparenting.
    REQUIRE(world_translation(child) == Xyz::Vector3F{3, 0, 0});
    world_translation(b);
    REQUIRE(a.world_version() == b.world_version());

    child.reparent(b);
    REQUIRE(child.parent() == &b);
    REQUIRE(a.children().empty());
    REQUIRE(world_translation(child) == Xyz::Vector3F{7, 0, 0});
}

TEST_CASE("Node: a removed child resolves as a root")
{
    Node parent;
    parent.set_local_transform(translate(1, 0, 0));
    auto& child = parent.add_child(std::make_unique<Node>());
    child.set_local_transform(translate(2, 0, 0));
    REQUIRE(world_translation(child) == Xyz::Vector3F{3, 0, 0});

    auto released = parent.remove_child(child);
    REQUIRE(released.get() == &child);
    REQUIRE(child.parent() == nullptr);
    REQUIRE(world_translation(child) == Xyz::Vector3F{2, 0, 0});
}

TEST_CASE("Node: structural mistakes throw")
{
    Node root;
    auto& child = root.add_child(std::make_unique<Node>());
    auto& grandchild = child.add_child(std::make_unique<Node>());

    // Reparenting a node below its own descendant would create a cycle.
    REQUIRE_THROWS_AS(child.reparent(grandchild), TungstenException);
    // A root has no parent to detach from.
    REQUIRE_THROWS_AS(root.reparent(child), TungstenException);
    // A node cannot be a child of two nodes.
    Node other;
    REQUIRE_THROWS_AS(other.remove_child(child), TungstenException);
    REQUIRE_THROWS_AS(root.add_child(nullptr), TungstenException);
}

TEST_CASE("Node: components know their owner")
{
    Node node;
    auto& component = node.add_component(std::make_unique<TestComponent>());
    REQUIRE(component.owner() == &node);
    REQUIRE(node.components().size() == 1);

    REQUIRE_THROWS_AS(node.add_component(nullptr), TungstenException);
}

TEST_CASE("Scene: owns roots and hands them back on remove")
{
    Scene scene;
    auto& root = scene.add(std::make_unique<Node>());
    REQUIRE(scene.roots().size() == 1);

    auto released = scene.remove(root);
    REQUIRE(released.get() == &root);
    REQUIRE(scene.roots().empty());
    REQUIRE_THROWS_AS(scene.remove(root), TungstenException);

    // A detached subtree moves freely between a node and the scene.
    Node other;
    other.add_child(std::move(released));
    scene.add(other.remove_child(root));
    REQUIRE(scene.roots().size() == 1);
    REQUIRE(root.parent() == nullptr);
}

TEST_CASE("Scene: the snapshot double-buffer swaps front and back")
{
    Scene scene;
    auto& snapshots = scene.snapshots();
    snapshots.back().time = 1.0f;
    snapshots.swap();
    REQUIRE(snapshots.front().time == 1.0f);
    REQUIRE(snapshots.back().time == 0.0f);
    snapshots.back().time = 2.0f;
    snapshots.swap();
    REQUIRE(snapshots.front().time == 2.0f);
    REQUIRE(snapshots.back().time == 1.0f);
}
