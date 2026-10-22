//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Model/ModelBuilder.hpp"

#include <memory>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/VertexLayoutBuilder.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"

using namespace Tungsten;

namespace
{
    // Fabricates GL ids, so that a model can be built and removed without a
    // GL context.
    class FakeOglWrapper : public DummyOglWrapper
    {
    public:
        void gen_buffers(GLsizei n, GLuint* buffers) override
        {
            for (GLsizei i = 0; i < n; ++i)
                buffers[i] = next_id_++;
        }

        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
            for (GLsizei i = 0; i < n; ++i)
                arrays[i] = next_id_++;
        }

    private:
        GLuint next_id_ = 1;
    };

    struct FakeGlSession
    {
        FakeGlSession()
        {
            previous_ = set_custom_ogl_wrapper(
                std::make_unique<FakeOglWrapper>());
        }

        ~FakeGlSession()
        {
            set_custom_ogl_wrapper(std::move(previous_));
        }

    private:
        std::unique_ptr<IOglWrapper> previous_;
    };

    struct Fixture
    {
        Fixture()
        {
            positions = resources.register_layout(
                VertexLayoutBuilder()
                    .add_attribute(AttributeSemantic::POSITION)
                    .build());
            textured = resources.register_layout(
                VertexLayoutBuilder()
                    .add_attribute(AttributeSemantic::POSITION)
                    .add_attribute(AttributeSemantic::TEX_COORD_0)
                    .build());
        }

        // First, so that it is in place before the resource manager touches
        // GL, and stays until it is gone.
        FakeGlSession session;
        ResourceManager resources;
        Scene scene;
        VertexLayoutRef positions;
        VertexLayoutRef textured;
    };

    // A triangle from (-1, 0, 0) to (1, 2, 0).
    const std::vector<float> TRIANGLE = {
        -1, 0, 0,
        1, 0, 0,
        0, 2, 0
    };

    const std::vector<uint32_t> TRIANGLE_INDICES = {0, 1, 2};
}

TEST_CASE("ModelBuilder: build gives the queued renderables their meshes")
{
    Fixture f;
    const auto parent = f.scene.create_node();
    ModelBuilder builder(f.resources, f.scene, parent);
    const auto material = builder.add_material({});
    const auto mesh = builder.add_mesh(
        builder.add_vertices(f.positions, TRIANGLE), TRIANGLE_INDICES);
    builder.add_renderable(builder.root(), mesh, material);

    const auto model = builder.build();

    REQUIRE(f.scene.parent(model.root) == parent);
    const auto& renderable =
        f.scene.get_component<RenderableComponent>(model.root);
    REQUIRE(renderable.mesh == builder.get_mesh(mesh));
    REQUIRE(renderable.material == material);
    REQUIRE(renderable.local_bounds.min == Xyz::Vector3F(-1, 0, 0));
    REQUIRE(renderable.local_bounds.max == Xyz::Vector3F(1, 2, 0));
    REQUIRE(f.resources.get_mesh(renderable.mesh).layout == f.positions);
    REQUIRE(f.resources.get_mesh(renderable.mesh).index_type
            == ElementIndexType::UINT32);

    REQUIRE(model.resources.meshes.size() == 1);
    REQUIRE(model.resources.materials.size() == 1);
    // One for the vertices, one for the indices.
    REQUIRE(model.resources.arenas.size() == 2);
}

TEST_CASE("ModelBuilder: the bounds are in the root's coordinate system")
{
    Fixture f;
    const auto parent = f.scene.create_node();
    f.scene.set_local_transform(parent, Transform{.translation = {100, 0, 0}});
    ModelBuilder builder(f.resources, f.scene, parent);
    const auto child = f.scene.create_node(builder.root());
    f.scene.set_local_transform(child, Transform{.translation = {0, 0, 5}});
    const auto mesh = builder.add_mesh(
        builder.add_vertices(f.positions, TRIANGLE), TRIANGLE_INDICES);
    builder.add_renderable(child, mesh, {});

    const auto model = builder.build();

    REQUIRE(model.bounds.min == Xyz::Vector3F(-1, 0, 5));
    REQUIRE(model.bounds.max == Xyz::Vector3F(1, 2, 5));
}

TEST_CASE("ModelBuilder: vertices of different sizes get different arenas")
{
    Fixture f;
    ModelBuilder builder(f.resources, f.scene);
    const std::vector<float> textured = {
        0, 0, 0, 0, 0,
        1, 0, 0, 1, 0,
        0, 1, 0, 0, 1
    };
    builder.add_mesh(builder.add_vertices(f.positions, TRIANGLE),
                     TRIANGLE_INDICES);
    builder.add_mesh(builder.add_vertices(f.positions, TRIANGLE),
                     TRIANGLE_INDICES);
    builder.add_mesh(builder.add_vertices(f.textured, textured),
                     TRIANGLE_INDICES);

    const auto model = builder.build();

    REQUIRE(model.resources.meshes.size() == 3);
    REQUIRE(model.resources.arenas.size() == 3);
    const auto& a = f.resources.get_mesh(builder.get_mesh(0));
    const auto& b = f.resources.get_mesh(builder.get_mesh(1));
    const auto& c = f.resources.get_mesh(builder.get_mesh(2));
    REQUIRE(a.streams[0].arena == b.streams[0].arena);
    REQUIRE(a.streams[0].arena != c.streams[0].arena);
    REQUIRE(c.layout == f.textured);
}

TEST_CASE("ModelBuilder: meshes can share vertices")
{
    Fixture f;
    ModelBuilder builder(f.resources, f.scene);
    const auto vertices = builder.add_vertices(f.positions, TRIANGLE);
    const auto triangles = builder.add_mesh(vertices, TRIANGLE_INDICES);
    const auto lines = builder.add_mesh(vertices, {0, 1, 1, 2, 2, 0});

    const auto model = builder.build();

    const auto& a = f.resources.get_mesh(builder.get_mesh(triangles));
    const auto& b = f.resources.get_mesh(builder.get_mesh(lines));
    REQUIRE(a.streams[0].arena == b.streams[0].arena);
    REQUIRE(a.streams[0].offset == b.streams[0].offset);
    REQUIRE(a.ebo.offset != b.ebo.offset);
}

TEST_CASE("ModelBuilder: a skinned renderable is never culled")
{
    Fixture f;
    ModelBuilder builder(f.resources, f.scene);
    const auto mesh = builder.add_mesh(
        builder.add_vertices(f.positions, TRIANGLE), TRIANGLE_INDICES);
    builder.add_renderable(builder.root(), mesh, {},
                           std::make_shared<const Skin>());

    const auto model = builder.build();

    const auto& renderable =
        f.scene.get_component<RenderableComponent>(model.root);
    REQUIRE(renderable.skin);
    REQUIRE(!renderable.local_bounds);
    // The bind pose still counts towards the model's bounds.
    REQUIRE(model.bounds.max == Xyz::Vector3F(1, 2, 0));
}

TEST_CASE("ModelBuilder: an unbuilt model is removed")
{
    Fixture f;
    NodeId root;
    {
        ModelBuilder builder(f.resources, f.scene);
        root = builder.root();
        builder.add_material({});
        builder.add_mesh(builder.add_vertices(f.positions, TRIANGLE),
                         TRIANGLE_INDICES);
        REQUIRE(f.scene.is_alive(root));
    }
    REQUIRE(!f.scene.is_alive(root));
}

TEST_CASE("ModelBuilder: remove_model removes the nodes and resources")
{
    Fixture f;
    auto model = [&]
    {
        ModelBuilder builder(f.resources, f.scene);
        const auto mesh = builder.add_mesh(
            builder.add_vertices(f.positions, TRIANGLE), TRIANGLE_INDICES);
        builder.add_renderable(builder.root(), mesh, {});
        return builder.build();
    }();
    const auto root = model.root;
    const auto mesh = model.resources.meshes[0];

    remove_model(model, f.scene, f.resources);

    REQUIRE(!f.scene.is_alive(root));
    REQUIRE_THROWS_AS(f.resources.get_mesh(mesh), TungstenException);
    REQUIRE(!model.root);
    REQUIRE(model.resources.empty());
}

TEST_CASE("ModelBuilder: invalid vertices and indices are rejected")
{
    Fixture f;
    ModelBuilder builder(f.resources, f.scene);

    SECTION("A partial vertex")
    {
        const std::vector<float> data = {0, 0, 0, 1};
        REQUIRE_THROWS_AS(builder.add_vertices(f.positions, data),
                          TungstenException);
    }

    SECTION("No positions")
    {
        const auto layout = f.resources.register_layout(
            VertexLayoutBuilder()
                .add_attribute(AttributeSemantic::NORMAL)
                .build());
        REQUIRE_THROWS_AS(builder.add_vertices(layout, TRIANGLE),
                          TungstenException);
    }

    SECTION("An index past the vertices")
    {
        const auto vertices = builder.add_vertices(f.positions, TRIANGLE);
        REQUIRE_THROWS_AS(builder.add_mesh(vertices, {0, 1, 3}),
                          TungstenException);
    }

    SECTION("No indices")
    {
        const auto vertices = builder.add_vertices(f.positions, TRIANGLE);
        REQUIRE_THROWS_AS(builder.add_mesh(vertices, {}), TungstenException);
    }
}
