//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/NodeHandle.hpp"

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

    // World matrices are only current after a resolve, so every positional
    // assertion goes through this rather than reading them directly.
    Xyz::Vector3F resolved_translation(Scene& scene, NodeId id)
    {
        scene.resolve_transforms();
        const auto& m = scene.world_matrix(id);
        return {m[0, 3], m[1, 3], m[2, 3]};
    }
}

TEST_CASE("Transform: the default is the identity")
{
    const auto m = Transform().make_matrix();
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

    const auto m = transform.make_matrix();
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
    transform.rotation = euler_rotation(Xyz::Constants<float>::PI / 2, 0, 0);

    // A quarter turn maps the x axis onto the y axis.
    const auto m = transform.make_matrix();
    REQUIRE_THAT((m[0, 0]), WithinAbs(0, 1e-6));
    REQUIRE_THAT((m[1, 0]), WithinAbs(1, 1e-6));
}

TEST_CASE("Transform: rotations compose as a quaternion product")
{
    Transform transform;
    const auto quarter_turn = z_rotation(Xyz::Constants<float>::PI / 2);
    transform.rotation = quarter_turn * quarter_turn;

    // Two quarter turns about z reverse both the x and the y axis.
    const auto m = transform.make_matrix();
    REQUIRE_THAT((m[0, 0]), WithinAbs(-1, 1e-6));
    REQUIRE_THAT((m[1, 1]), WithinAbs(-1, 1e-6));
    REQUIRE_THAT((m[2, 2]), WithinAbs(1, 1e-6));
}

TEST_CASE("Scene: a root's world matrix is its local matrix")
{
    Scene scene;
    auto node = scene.add_node();
    node.set_local_transform(translate(1, 2, 3));

    REQUIRE(resolved_translation(scene, node.id())
            == Xyz::Vector3F{1, 2, 3});
}

TEST_CASE("Scene: a node's world matrix is the identity before the first"
          " resolve")
{
    Scene scene;
    auto node = scene.add_node();
    node.set_local_transform(translate(1, 2, 3));

    const auto& m = node.world_matrix();
    REQUIRE(m[0, 3] == 0.0f);
    REQUIRE(m[0, 0] == 1.0f);
}

TEST_CASE("Scene: children inherit their parent's world transform")
{
    Scene scene;
    auto parent = scene.add_node();
    parent.set_local_transform(translate(1, 0, 0));
    auto child = parent.add_child();
    child.set_local_transform(translate(2, 0, 0));

    REQUIRE(resolved_translation(scene, child.id())
            == Xyz::Vector3F{3, 0, 0});
}

TEST_CASE("Scene: moving an ancestor moves its whole subtree")
{
    Scene scene;
    auto root = scene.add_node();
    auto mid = root.add_child();
    auto leaf = mid.add_child();
    mid.set_local_transform(translate(1, 0, 0));
    leaf.set_local_transform(translate(1, 0, 0));

    REQUIRE(resolved_translation(scene, leaf.id())
            == Xyz::Vector3F{2, 0, 0});

    root.set_local_transform(translate(10, 0, 0));
    REQUIRE(resolved_translation(scene, leaf.id())
            == Xyz::Vector3F{12, 0, 0});
}

TEST_CASE("Scene: the resolve order puts parents before children whatever"
          " order the nodes were created in")
{
    Scene scene;
    // Created leaf-first, so the slot order is the reverse of the depth
    // order. Only the rebuilt order makes the single linear pass correct.
    auto c = scene.add_node();
    auto b = scene.add_node();
    auto a = scene.add_node();
    a.set_local_transform(translate(1, 0, 0));
    b.set_local_transform(translate(2, 0, 0));
    c.set_local_transform(translate(4, 0, 0));

    b.reparent(a);
    c.reparent(b);

    REQUIRE(resolved_translation(scene, c.id()) == Xyz::Vector3F{7, 0, 0});
}

TEST_CASE("Scene: reparenting picks up the new parent's transform")
{
    Scene scene;
    auto a = scene.add_node();
    auto b = scene.add_node();
    a.set_local_transform(translate(1, 0, 0));
    b.set_local_transform(translate(5, 0, 0));
    auto child = a.add_child();
    child.set_local_transform(translate(2, 0, 0));

    REQUIRE(resolved_translation(scene, child.id())
            == Xyz::Vector3F{3, 0, 0});

    child.reparent(b);
    REQUIRE(child.parent() == b);
    REQUIRE(a.children().empty());
    REQUIRE(resolved_translation(scene, child.id())
            == Xyz::Vector3F{7, 0, 0});
}

TEST_CASE("Scene: reparenting to a null handle makes the node a root")
{
    Scene scene;
    auto parent = scene.add_node();
    parent.set_local_transform(translate(1, 0, 0));
    auto child = parent.add_child();
    child.set_local_transform(translate(2, 0, 0));
    REQUIRE(resolved_translation(scene, child.id())
            == Xyz::Vector3F{3, 0, 0});

    child.reparent({});
    REQUIRE(!child.parent().id());
    REQUIRE(resolved_translation(scene, child.id())
            == Xyz::Vector3F{2, 0, 0});
}

TEST_CASE("Scene: children are iterated in the order they were added")
{
    Scene scene;
    auto parent = scene.add_node();
    const auto first = parent.add_child().id();
    const auto second = parent.add_child().id();
    const auto third = parent.add_child().id();

    std::vector<NodeId> seen;
    for (const NodeId child : parent.children())
        seen.push_back(child);

    REQUIRE(seen == std::vector{first, second, third});
}

TEST_CASE("Scene: structural mistakes throw")
{
    Scene scene;
    auto root = scene.add_node();
    auto child = root.add_child();
    auto grandchild = child.add_child();

    // Reparenting a node below its own descendant would create a cycle.
    REQUIRE_THROWS_AS(child.reparent(grandchild), TungstenException);
    // So would reparenting a node under itself.
    REQUIRE_THROWS_AS(child.reparent(child), TungstenException);
    REQUIRE_THROWS_AS(scene.world_matrix(NodeId{}), TungstenException);
}

TEST_CASE("Scene: removing a node removes its whole subtree")
{
    Scene scene;
    auto root = scene.add_node();
    auto child = root.add_child();
    auto grandchild = child.add_child();
    const auto child_id = child.id();
    const auto grandchild_id = grandchild.id();

    child.remove();

    REQUIRE(!scene.is_alive(child_id));
    REQUIRE(!scene.is_alive(grandchild_id));
    REQUIRE(scene.is_alive(root.id()));
    REQUIRE(root.children().empty());
    // The resolve pass must no longer visit them.
    scene.resolve_transforms();
}

TEST_CASE("Scene: removing a node drops the components of its whole subtree")
{
    Scene scene;
    auto root = scene.add_node();
    auto child = root.add_child();
    auto grandchild = child.add_child();
    root.add(RenderableComponent{});
    child.add(RenderableComponent{});
    grandchild.add(LightComponent{});
    REQUIRE(scene.components<RenderableComponent>().items.size() == 2);

    child.remove();

    REQUIRE(scene.components<RenderableComponent>().items.size() == 1);
    REQUIRE(scene.components<RenderableComponent>().owners[0] == root.id());
    REQUIRE(scene.components<LightComponent>().items.empty());
}

TEST_CASE("Scene: an id to a removed node stops resolving")
{
    Scene scene;
    const auto id = scene.add_node().id();
    scene.remove(id);

    REQUIRE(!scene.is_alive(id));
    REQUIRE_THROWS_AS(scene.world_matrix(id), TungstenException);
    REQUIRE_THROWS_AS(scene.remove(id), TungstenException);

    // The slot is reused, but the generation bump keeps the old id from
    // naming whoever takes it over.
    const auto reused = scene.add_node().id();
    REQUIRE(reused.index == id.index);
    REQUIRE(reused != id);
    REQUIRE(scene.is_alive(reused));
    REQUIRE(!scene.is_alive(id));
}

TEST_CASE("Scene: components attach to a node and are found by kind")
{
    Scene scene;
    auto node = scene.add_node();
    auto other = scene.add_node();

    node.add(LightComponent{.intensity = 4.0f});

    REQUIRE(node.get<LightComponent>().intensity == 4.0f);
    REQUIRE(node.find<RenderableComponent>() == nullptr);
    REQUIRE(other.find<LightComponent>() == nullptr);
    REQUIRE_THROWS_AS(other.get<LightComponent>(), TungstenException);

    node.remove_component<LightComponent>();
    REQUIRE(node.find<LightComponent>() == nullptr);
    REQUIRE(scene.components<LightComponent>().items.empty());
}

TEST_CASE("Scene: attaching a component to a dead node throws")
{
    Scene scene;
    const auto id = scene.add_node().id();
    scene.remove(id);

    REQUIRE_THROWS_AS(scene.add_component(id, LightComponent{}),
                      TungstenException);
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