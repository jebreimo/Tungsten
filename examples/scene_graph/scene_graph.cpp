//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// The scene-graph pipeline (docs/scene_graph_design.md) end to end: a hub
// cube with two orbiting child cubes — one opaque, one transparent — a
// directional light, an orbiting point light, and a perspective camera, drawn
// each frame through
//
//     update transforms
//     scene.resolve_transforms()
//     SnapshotBuilder::build(scene, camera_node, snapshots.back())
//     snapshots.swap()
//     Renderer::render(snapshots.front())
//
// The children are plain child nodes: their orbit is nothing but the hub's
// rotation composed into them by the resolve pass.

#include <cmath>
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>
#include <Xyz/Mesh/BuildMesh.hpp>

namespace
{
    using namespace Tungsten;

    Xyz::BBox3F unit_box()
    {
        return {{-1, -1, -1}, {1, 1, 1}};
    }

    std::pair<std::vector<uint16_t>, std::vector<float>> make_cube_mesh()
    {
        std::vector<uint16_t> indexes;
        std::vector<float> vertexes;

        using Builder2F = Xyz::MeshAttributeBuilder<Xyz::Vector2F, std::vector<float>>;
        using Builder3F = Xyz::MeshAttributeBuilder<Xyz::Vector3F, std::vector<float>>;

        Xyz::MeshBuilder builder{
            .indexes = Xyz::MeshIndexBuilder(indexes),
            .coords = Builder3F(vertexes, 8),
            .normals = std::optional(Builder3F(vertexes, 8, 3)),
            .tex_coords = std::optional(Builder2F(vertexes, 8, 6))
        };

        Xyz::OrientedCuboid<float> cuboid{{{-1, -1, -1}, {0, 0, 0}}, {2, 2, 2}};
        Xyz::build_mesh(builder, cuboid);
        return {std::move(indexes), std::move(vertexes)};
    }

    class SceneGraphLoop : public EventLoop
    {
    public:
        explicit SceneGraphLoop(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            register_builtin_shader_families(resources_);
            const auto shader = resources_.register_shader_variant(
                {BLINN_PHONG_FAMILY, 0});

            const auto gold = make_material(
                shader, false,
                make_blinn_phong_material_params(StandardColorMaterial::GOLD));
            const auto slate = make_material(
                shader, false,
                make_blinn_phong_material_params(StandardColorMaterial::SLATE));
            const auto glass = make_material(
                shader, true,
                make_blinn_phong_material_params(StandardColorMaterial::COPPER,
                                                 0.4f));

            const auto mesh = make_cube_mesh();

            hub_ = scene_.add_node();
            add_renderable(hub_, mesh, gold);
            left_ = add_child_cube(mesh, glass, -2.2f);
            right_ = add_child_cube(mesh, slate, 2.2f);

            camera_ = scene_.add_node();
            Transform camera_transform;
            camera_transform.translation = {0, 0, 8};
            camera_.set_local_transform(camera_transform);
            camera_.add(CameraComponent{
                .near_plane = 0.5f,
                .far_plane = 50.0f,
                .aspect = app.viewport().aspect_ratio()
            });

            auto light_node = scene_.add_node();
            Transform light_transform;
            // A directional light shines along its node's -z axis; tilt the
            // node so the light comes in from the upper right.
            light_transform.rotation = euler_rotation(-0.6f, 0.4f, 0.0f);
            light_node.set_local_transform(light_transform);
            light_node.add(LightComponent{
                .type = LightType::DIRECTIONAL,
                .color = {1.0f, 0.97f, 0.9f},
                .intensity = 1.0f
            });

            // A point light, to contrast with the directional one: it has a
            // position rather than only a direction, and its contribution
            // falls off as 1/d^2, windowed to reach zero at `range`. It orbits
            // the cubes in on_update so the falloff is visible as a highlight
            // travelling across them.
            point_light_ = scene_.add_node();
            point_light_.add(LightComponent{
                .type = LightType::POINT,
                .color = {1.0f, 0.55f, 0.2f},
                // Chosen so the light reads clearly at its closest approach
                // without clipping the cubes to flat colour: at ~2 units the
                // 1/d^2 falloff already scales this down to roughly the
                // directional light's contribution.
                .intensity = 3.0f,
                .range = 8.0f
            });

            std::cout << get_device_info() << '\n';
            set_swap_interval(app, SwapInterval::VSYNC);
        }

        bool on_event(const SDL_Event& event) override
        {
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                // Through the handle, not a cached pointer: the component
                // arrays move their contents as they grow.
                camera_.get<CameraComponent>().aspect =
                    application().viewport().aspect_ratio();
                return true;
            }
            return false;
        }

        void on_update() override
        {
            const auto t = float(SDL_GetTicks() - start_ticks_) / 1000.0f;

            // Rotating the hub is all it takes to orbit the children: the
            // next resolve composes their world matrices from it (§2).
            Transform hub;
            hub.rotation = euler_rotation(0.4f * t, 0.2f * t, 0.0f);
            hub_.set_local_transform(hub);

            spin_child(left_, -2.2f, 1.7f * t);
            spin_child(right_, 2.2f, -1.3f * t);

            // Only the point light's position matters — it has no meaningful
            // orientation — so moving it is enough to sweep its highlight
            // across the cubes.
            Transform point;
            point.translation = {
                3.2f * std::cos(0.9f * t), 1.5f,
                3.2f * std::sin(0.9f * t)
            };
            point_light_.set_local_transform(point);
        }

        void on_draw() override
        {
            set_face_culling_enabled(true);
            set_clear_color({0.12f, 0.13f, 0.16f, 1.0f});
            clear(ClearBits::COLOR_DEPTH);
            const auto viewport = application().viewport();
            set_viewport(viewport);
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            auto& snapshots = scene_.snapshots();
            builder_.build(scene_, camera_.id(), snapshots.back());
            snapshots.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots.back().ambient_light = {0.35f, 0.35f, 0.38f};
            snapshots.swap();

            renderer_.render(snapshots.front());

            // Single-threaded: the frame just drawn is complete (§11).
            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
            set_ogl_tracing_enabled(false);
        }

    private:
        MaterialRef make_material(ShaderProgramRef shader, bool transparent,
                                  std::vector<std::byte> params)
        {
            Material material;
            material.shader = shader;
            material.parameter_data = std::move(params);
            material.transparent = transparent;
            return resources_.create_material(std::move(material));
        }

        MeshRef make_cube_mesh()
        {
            auto [indexes, vertexes] = ::make_cube_mesh();

            constexpr size_t STRIDE = 8; // 3 coords + 3 normals + 2 tex coords

            const auto layout = resources_.register_layout(VertexLayoutBuilder()
                .add_attribute(AttributeSemantic::POSITION)
                .add_attribute(AttributeSemantic::NORMAL)
                .add_attribute(AttributeSemantic::TEX_COORD_0)
                .build());
            vbo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, STRIDE * sizeof(float),
                uint32_t(vertexes.size() / STRIDE));
            ebo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, sizeof(uint16_t), uint32_t(indexes.size()));

            const auto vertices = resources_.allocate(
                vbo_arena_, uint32_t(vertexes.size() / STRIDE));
            const auto indices = resources_.allocate(
                ebo_arena_, uint32_t(indexes.size()));

            // Rebase to absolute indices (§3): the portable path has no
            // baseVertex draw argument.
            for (auto& index : indexes)
                index = uint16_t(index + vertices.offset);

            resources_.upload(vertices, vertexes.data(),
                              vertexes.size() * sizeof(float));
            resources_.upload(indices, indexes.data(),
                              indexes.size() * sizeof(uint16_t));

            Mesh mesh;
            const BufferArenaRef vbos[] = {vbo_arena_};
            mesh.vao = resources_.get_vao(vbos, ebo_arena_, layout);
            mesh.streams = {vertices};
            mesh.layout = layout;
            mesh.ebo = indices;
            mesh.index_type = ElementIndexType::UINT16;
            mesh.primitive = TopologyType::TRIANGLES;
            return resources_.create_mesh(std::move(mesh));
        }

        static void add_renderable(NodeHandle node, MeshRef mesh,
                                   MaterialRef material)
        {
            node.add(RenderableComponent{
                .mesh = mesh,
                .material = material,
                .local_bounds = unit_box()
            });
        }

        NodeHandle add_child_cube(MeshRef mesh, MaterialRef material, float x)
        {
            auto child = hub_.add_child();
            spin_child(child, x, 0);
            add_renderable(child, mesh, material);
            return child;
        }

        static void spin_child(NodeHandle child, float x, float angle)
        {
            Transform transform;
            transform.translation = {x, 0, 0};
            transform.rotation = euler_rotation(angle, 0.5f * angle, 0);
            transform.scale = {0.6f, 0.6f, 0.6f};
            child.set_local_transform(transform);
        }

        ResourceManager resources_;
        Scene scene_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        BufferArenaRef vbo_arena_;
        BufferArenaRef ebo_arena_;
        NodeHandle hub_;
        NodeHandle left_;
        NodeHandle right_;
        NodeHandle camera_;
        NodeHandle point_light_;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char** argv)
{
    try
    {
        argos::ArgumentParser parser;
        SdlApplication::add_command_line_options(parser);
        const auto args = parser.parse(argc, argv);
        SdlApplication app("SceneGraph");
        app.read_command_line_options(args);
        set_ogl_tracing_enabled(true);
        app.run<SceneGraphLoop>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
