//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Rendering/SnapshotBuilder.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Tungsten/SceneGraph/CameraComponent.hpp"
#include "Tungsten/SceneGraph/LightComponent.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/SceneGraph/NodeHandle.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"

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

    Xyz::BBox3F unit_box()
    {
        Xyz::BBox3F box;
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
            camera_node = scene.add_node();
            camera_node.add(CameraComponent{
                .mode = ProjectionMode::ORTHOGRAPHIC,
                .near_plane = 0.1f,
                .far_plane = 100.0f,
                .ortho_size = 5.0f
            });
        }

        // Adds a root node with a renderable at the given position.
        NodeHandle add_renderable(float x, float y, float z)
        {
            auto node = scene.add_node();
            node.set_local_transform(translate(x, y, z));
            node.add(RenderableComponent{
                .mesh = mesh,
                .material = material,
                .local_bounds = unit_box()
            });
            return node;
        }

        void build()
        {
            scene.resolve_transforms();
            SnapshotBuilder(resources)
                .build(scene, camera_node.id(), snapshot);
        }

        ResourceManager resources;
        Scene scene;
        NodeHandle camera_node;
        RenderSnapshot snapshot;
        MeshRef mesh;
        MaterialRef material;
    };
}

TEST_CASE("Scene: resolve_transforms resolves world matrices through the hierarchy")
{
    Scene scene;
    auto root = scene.add_node();
    auto child = root.add_child();
    root.set_local_transform(translate(1, 0, 0));
    child.set_local_transform(translate(5, 0, 0));

    scene.resolve_transforms();
    REQUIRE((child.world_matrix()[0, 3]) == 6.0f);

    // Moving the parent moves the child on the next resolve.
    root.set_local_transform(translate(-1, 0, 0));
    scene.resolve_transforms();
    REQUIRE((child.world_matrix()[0, 3]) == 4.0f);
}

TEST_CASE("SnapshotBuilder: fills in the camera state")
{
    Bench bench;
    bench.camera_node.set_local_transform(translate(0, 0, 10));

    bench.build();

    REQUIRE(bench.snapshot.camera_position == Xyz::Vector3F{0, 0, 10});
    // The view matrix is the inverse of the camera node's world transform.
    REQUIRE_THAT((bench.snapshot.view_matrix[2, 3]), WithinAbs(-10, 1e-6));
}

TEST_CASE("SnapshotBuilder: throws if the camera node has no camera")
{
    Bench bench;
    const auto bare = bench.scene.add_node().id();

    REQUIRE_THROWS_AS(
        SnapshotBuilder(bench.resources)
            .build(bench.scene, bare, bench.snapshot),
        TungstenException);
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
    auto node = bench.scene.add_node();
    node.set_local_transform(translate(1000, 0, 0)); // far outside
    node.add(RenderableComponent{
        .mesh = bench.mesh,
        .material = bench.material
    }); // local_bounds left empty

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
}

TEST_CASE("SnapshotBuilder: skips invisible and incomplete renderables")
{
    Bench bench;
    const auto invisible = bench.add_renderable(0, 0, -5);
    invisible.get<RenderableComponent>().visible = false;

    auto no_mesh = bench.scene.add_node();
    no_mesh.add(RenderableComponent{
        .material = bench.material // mesh ref left null
    });

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

    const auto node = bench.add_renderable(0, 0, -5);
    node.get<RenderableComponent>().material = transparent_material;
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
    const auto near = bench.add_renderable(0, 0, -5);
    const auto far = bench.add_renderable(0, 0, -50);
    near.get<RenderableComponent>().material = transparent_material;
    far.get<RenderableComponent>().material = transparent_material;

    bench.build();
    const auto& items = bench.snapshot.transparent_items;
    REQUIRE(items.size() == 2);
    // items[0] is the near one; the far one must sort (and so draw) first.
    REQUIRE(items[1].sort_key() < items[0].sort_key());
}

TEST_CASE("SnapshotBuilder: the render layer dominates the sort key")
{
    Bench bench;
    // Near, but on a higher layer.
    const auto overlay = bench.add_renderable(0, 0, -5);
    overlay.get<RenderableComponent>().render_layer = 1;
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
    auto node = bench.scene.add_node();
    node.set_local_transform(translate(1, 2, 3));
    node.add(LightComponent{
        .type = LightType::POINT,
        .color = {1, 0.5f, 0.25f},
        .intensity = 2.0f,
        .range = 20.0f
    });

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

TEST_CASE("LightData: the type is stored as the float the shader converts")
{
    // The shader reads position.w and *converts* it — int(light.position.w) —
    // so the slot has to hold 0.0, 1.0 or 2.0. Asserting through type() alone
    // would not catch an encoding that round-trips in C++ but reaches the
    // shader as a denormal.
    LightData data;

    data.set_type(LightType::DIRECTIONAL);
    REQUIRE(data.data()[3] == 0.0f);
    REQUIRE(data.type() == LightType::DIRECTIONAL);

    data.set_type(LightType::POINT);
    REQUIRE(data.data()[3] == 1.0f);
    REQUIRE(data.type() == LightType::POINT);

    data.set_type(LightType::SPOT);
    REQUIRE(data.data()[3] == 2.0f);
    REQUIRE(data.type() == LightType::SPOT);
}

TEST_CASE("SnapshotBuilder: a point light reaches the snapshot as a point light")
{
    Bench bench;
    auto node = bench.scene.add_node();
    node.add(LightComponent{.type = LightType::POINT});

    bench.build();
    REQUIRE(bench.snapshot.lights.size() == 1);
    REQUIRE(bench.snapshot.lights[0].data()[3] == 1.0f);
}

TEST_CASE("SnapshotBuilder: a removed node's renderable stops being drawn")
{
    Bench bench;
    bench.add_renderable(0, 0, -5);
    const auto doomed = bench.add_renderable(1, 0, -5);

    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 2);

    doomed.remove();
    bench.build();
    REQUIRE(bench.snapshot.opaque_items.size() == 1);
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
