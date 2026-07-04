//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/SnapshotBuilder.hpp"

#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Tungsten/Neo/CameraComponent.hpp"
#include "Tungsten/Neo/LightComponent.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/Scene.hpp"
#include "Tungsten/Neo/TransformUpdater.hpp"

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

    AABB unit_box()
    {
        AABB box;
        box.min = {-1, -1, -1};
        box.max = {1, 1, 1};
        return box;
    }

    // A scene-graph test bench: a resource manager with one mesh and one
    // material (creating them touches no GL), a scene, and an orthographic
    // camera at the origin looking along -z with a 10x10x[0.1, 100] view
    // volume.
    struct Bench
    {
        Bench()
        {
            mesh = resources.create_mesh(Mesh{});
            material = resources.create_material(Material{});
            camera.mode = ProjectionMode::ORTHOGRAPHIC;
            camera.ortho_size = 5.0f;
            camera.near_plane = 0.1f;
            camera.far_plane = 100.0f;
        }

        // Adds a root node with a renderable at the given position.
        Node& add_renderable(float x, float y, float z)
        {
            auto& node = scene.add(std::make_unique<Node>());
            node.set_local_transform(translate(x, y, z));
            auto component = std::make_unique<RenderableComponent>();
            component->mesh = mesh;
            component->material = material;
            component->local_bounds = unit_box();
            node.add_component(std::move(component));
            return node;
        }

        void build()
        {
            TransformUpdater::resolve(scene);
            SnapshotBuilder(resources).build(scene, camera, snapshot);
        }

        ResourceManager resources;
        Scene scene;
        CameraComponent camera;
        RenderSnapshot snapshot;
        MeshRef mesh;
        MaterialRef material;
    };
}

TEST_CASE("TransformUpdater: bounds propagate up through the tree")
{
    Scene scene;
    auto& root = scene.add(std::make_unique<Node>());
    auto& child = root.add_child(std::make_unique<Node>());
    child.set_local_transform(translate(5, 0, 0));
    auto component = std::make_unique<RenderableComponent>();
    component->local_bounds = unit_box();
    child.add_component(std::move(component));

    TransformUpdater::resolve(scene);
    REQUIRE(child.world_bounds().min == Xyz::Vector3F{4, -1, -1});
    REQUIRE(child.world_bounds().max == Xyz::Vector3F{6, 1, 1});
    // The parent has no renderable of its own; its bounds are the child's.
    REQUIRE(root.world_bounds().min == Xyz::Vector3F{4, -1, -1});
    REQUIRE(root.world_bounds().max == Xyz::Vector3F{6, 1, 1});

    // Moving the child moves the ancestors' bounds on the next resolve.
    child.set_local_transform(translate(-5, 0, 0));
    TransformUpdater::resolve(scene);
    REQUIRE(root.world_bounds().min == Xyz::Vector3F{-6, -1, -1});
    REQUIRE(root.world_bounds().max == Xyz::Vector3F{-4, 1, 1});
}

TEST_CASE("TransformUpdater: a subtree without renderables has empty bounds")
{
    Scene scene;
    auto& root = scene.add(std::make_unique<Node>());
    root.add_child(std::make_unique<Node>());

    TransformUpdater::resolve(scene);
    REQUIRE(root.world_bounds().is_empty());
}

TEST_CASE("SnapshotBuilder: fills in the camera state")
{
    Bench bench;
    auto& camera_node = bench.scene.add(std::make_unique<Node>());
    camera_node.set_local_transform(translate(0, 0, 10));
    auto camera = std::make_unique<CameraComponent>();
    camera->mode = ProjectionMode::ORTHOGRAPHIC;
    camera->ortho_size = 5.0f;
    auto& camera_ref = dynamic_cast<CameraComponent&>(
        camera_node.add_component(std::move(camera)));

    TransformUpdater::resolve(bench.scene);
    SnapshotBuilder(bench.resources)
        .build(bench.scene, camera_ref, bench.snapshot);

    REQUIRE(bench.snapshot.camera_position == Xyz::Vector3F{0, 0, 10});
    // The view matrix is the inverse of the camera node's world transform.
    REQUIRE_THAT((bench.snapshot.view_matrix[2, 3]), WithinAbs(-10, 1e-6));
}

TEST_CASE("SnapshotBuilder: extracts visible renderables inside the frustum")
{
    Bench bench;
    bench.add_renderable(0, 0, -5);
    bench.add_renderable(2, 2, -50);

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 2);
    REQUIRE(bench.snapshot.transparent_items.empty());
    REQUIRE(bench.snapshot.opaque_items[0].mesh() == bench.mesh);
    REQUIRE(bench.snapshot.opaque_items[0].material() == bench.material);

    const auto world = bench.snapshot.opaque_items[0].world_transform();
    REQUIRE_THAT((world[2, 3]), WithinAbs(-5, 1e-6));
}

TEST_CASE("SnapshotBuilder: culls items outside the frustum")
{
    Bench bench;
    bench.add_renderable(0, 0, -5);    // inside
    bench.add_renderable(100, 0, -5);  // far off to the right
    bench.add_renderable(0, 0, 50);    // behind the camera
    bench.add_renderable(0, 0, -200);  // beyond the far plane

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
}

TEST_CASE("SnapshotBuilder: an item with empty bounds is never culled")
{
    Bench bench;
    auto& node = bench.scene.add(std::make_unique<Node>());
    node.set_local_transform(translate(1000, 0, 0)); // far outside
    auto component = std::make_unique<RenderableComponent>();
    component->mesh = bench.mesh;
    component->material = bench.material;
    node.add_component(std::move(component)); // local_bounds left empty

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
}

TEST_CASE("SnapshotBuilder: skips invisible and incomplete renderables")
{
    Bench bench;
    auto& invisible = bench.add_renderable(0, 0, -5);
    dynamic_cast<RenderableComponent&>(*invisible.components()[0])
        .visible = false;

    auto& no_mesh = bench.scene.add(std::make_unique<Node>());
    auto component = std::make_unique<RenderableComponent>();
    component->material = bench.material; // mesh ref left null
    no_mesh.add_component(std::move(component));

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.empty());
}

TEST_CASE("SnapshotBuilder: transparent materials go to the transparent list")
{
    Bench bench;
    Material material;
    material.transparent = true;
    const auto transparent_material = bench.resources.create_material(
        std::move(material));

    auto& node = bench.add_renderable(0, 0, -5);
    dynamic_cast<RenderableComponent&>(*node.components()[0])
        .material = transparent_material;
    bench.add_renderable(0, 0, -7);

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
    REQUIRE(bench.snapshot.transparent_items.size() == 1);
}

TEST_CASE("SnapshotBuilder: opaque sort keys order front-to-back")
{
    Bench bench;
    bench.add_renderable(0, 0, -50);
    bench.add_renderable(0, 0, -5);

    bench.build();
    const auto& items = bench.snapshot.opaque_items;
    REQUIRE(items.size() == 2);
    // items[0] is the far one; the near one must sort first.
    REQUIRE(items[1].sort_key() < items[0].sort_key());
}

TEST_CASE("SnapshotBuilder: transparent sort keys order back-to-front")
{
    Bench bench;
    Material material;
    material.transparent = true;
    const auto transparent_material = bench.resources.create_material(
        std::move(material));
    auto& near = bench.add_renderable(0, 0, -5);
    auto& far = bench.add_renderable(0, 0, -50);
    dynamic_cast<RenderableComponent&>(*near.components()[0])
        .material = transparent_material;
    dynamic_cast<RenderableComponent&>(*far.components()[0])
        .material = transparent_material;

    bench.build();
    const auto& items = bench.snapshot.transparent_items;
    REQUIRE(items.size() == 2);
    // items[0] is the near one; the far one must sort (and so draw) first.
    REQUIRE(items[1].sort_key() < items[0].sort_key());
}

TEST_CASE("SnapshotBuilder: the render layer dominates the sort key")
{
    Bench bench;
    auto& overlay = bench.add_renderable(0, 0, -5); // near, but higher layer
    dynamic_cast<RenderableComponent&>(*overlay.components()[0])
        .render_layer = 1;
    bench.add_renderable(0, 0, -50);

    bench.build();
    const auto& items = bench.snapshot.opaque_items;
    REQUIRE(items.size() == 2);
    // items[0] is the overlay: it must sort after the layer-0 item despite
    // being nearer.
    REQUIRE(items[0].sort_key() > items[1].sort_key());
}

TEST_CASE("SnapshotBuilder: extracts lights with node position and direction")
{
    Bench bench;
    auto& node = bench.scene.add(std::make_unique<Node>());
    node.set_local_transform(translate(1, 2, 3));
    auto light = std::make_unique<LightComponent>();
    light->type = LightType::POINT;
    light->color = {1, 0.5f, 0.25f};
    light->intensity = 2.0f;
    light->range = 20.0f;
    node.add_component(std::move(light));

    bench.build();
    REQUIRE(bench.snapshot.lights.size() == 1);
    const auto& data = bench.snapshot.lights[0];
    REQUIRE(data.type() == LightType::POINT);
    REQUIRE(data.position() == Xyz::Vector3F{1, 2, 3});
    // An unrotated node shines along -z, like the camera convention.
    REQUIRE_THAT(data.direction()[2], WithinAbs(-1, 1e-6));
    REQUIRE(data.color() == Xyz::Vector3F{1, 0.5f, 0.25f});
    REQUIRE(data.intensity() == 2.0f);
    REQUIRE(data.range() == 20.0f);
}

TEST_CASE("SnapshotBuilder: the normal matrix is identity for rigid motion")
{
    Bench bench;
    bench.add_renderable(3, 4, -5);

    bench.build();
    const auto normal = bench.snapshot.opaque_items[0].normal_matrix();
    for (size_t row = 0; row < 3; ++row)
    {
        for (size_t col = 0; col < 3; ++col)
            REQUIRE_THAT((normal[row, col]),
                         WithinAbs(row == col ? 1 : 0, 1e-6));
    }
}

TEST_CASE("SnapshotBuilder: rebuilding reuses the snapshot")
{
    Bench bench;
    bench.add_renderable(0, 0, -5);

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
    // A second build must not accumulate.
    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
}
