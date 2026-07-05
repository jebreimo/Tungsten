//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// The scene-graph pipeline (docs/scene_graph_design.md) end to end: a hub
// cube with two orbiting child cubes — one opaque, one transparent — a
// directional light, and a perspective camera, drawn each frame through
//
//     update transforms
//     TransformUpdater::resolve(scene)
//     SnapshotBuilder::build(scene, camera, snapshots.back())
//     snapshots.swap()
//     Renderer::render(snapshots.front())
//
// The children are plain child nodes: their orbit is nothing but the hub's
// rotation propagating down through the dirty-transform machinery.

#include <cstring>
#include <iostream>
#include <Argos/Argos.hpp>

// Deliberately not <Tungsten/Tungsten.hpp>: the umbrella still exports the
// old shader layer, whose ShaderProgram and VertexAttribute the scene-graph
// path replaces (§9) — the names collide until the old layer is retired.
#include <Tungsten/DeviceInfo.hpp>
#include <Tungsten/Gl/GlRendering.hpp>
#include <Tungsten/Gl/GlStateManagement.hpp>
#include <Tungsten/MeshDataBuilder.hpp>
#include <Tungsten/MeshUtilities.hpp>
#include <Tungsten/Sdl/EventLoop.hpp>
#include <Tungsten/Sdl/SdlApplication.hpp>

#include "Tungsten/Gl/IOglWrapper.hpp"
#include "Tungsten/Neo/BuiltinShaders.hpp"
#include "Tungsten/Neo/CameraComponent.hpp"
#include "Tungsten/Neo/LightComponent.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/Renderer.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/Scene.hpp"
#include "Tungsten/Neo/SnapshotBuilder.hpp"
#include "Tungsten/Neo/TransformUpdater.hpp"
#include "Tungsten/Render/ColorMaterials.hpp"

namespace
{
    using namespace Tungsten;

    // The std140 MaterialBlock of the builtin BlinnPhong family
    // (Shaders/BlinnPhong-frag.glsl): ambient, diffuse (w = opacity),
    // specular (w = shininess), and the map flags (unused here).
    std::vector<uint8_t> make_material_params(const ColorMaterial& material,
                                              float opacity)
    {
        const float values[12] = {
            material.ambient[0], material.ambient[1], material.ambient[2], 0,
            material.diffuse[0], material.diffuse[1], material.diffuse[2], opacity,
            material.specular[0], material.specular[1], material.specular[2], material.shininess
        };
        std::vector<uint8_t> blob(16 * sizeof(float), 0);
        std::memcpy(blob.data(), values, sizeof(values));
        return blob;
    }

    AABB unit_box()
    {
        return {{-1, -1, -1}, {1, 1, 1}};
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
                make_material_params(get_standard_color_material(StandardColorMaterial::GOLD),
                                     1.0f));
            const auto slate = make_material(
                shader, false,
                make_material_params(get_standard_color_material(StandardColorMaterial::SLATE),
                                     1.0f));
            const auto glass = make_material(
                shader, true,
                make_material_params(get_standard_color_material(StandardColorMaterial::COPPER),
                                     0.4f));

            const auto mesh = make_cube_mesh();

            hub_ = &scene_.add(std::make_unique<Node>());
            add_renderable(*hub_, mesh, gold);
            left_ = &add_child_cube(mesh, glass, -2.2f);
            right_ = &add_child_cube(mesh, slate, 2.2f);

            auto& camera_node = scene_.add(std::make_unique<Node>());
            Transform camera_transform;
            camera_transform.translation = {0, 0, 8};
            camera_node.set_local_transform(camera_transform);
            auto camera = std::make_unique<CameraComponent>();
            camera->near_plane = 0.5f;
            camera->far_plane = 50.0f;
            camera->aspect = app.viewport().aspect_ratio();
            camera_ = static_cast<CameraComponent*>(
                &camera_node.add_component(std::move(camera)));

            auto& light_node = scene_.add(std::make_unique<Node>());
            Transform light_transform;
            // A directional light shines along its node's -z axis; tilt the
            // node so the light comes in from the upper right.
            light_transform.rotation = Xyz::Orientation3F(-0.6f, 0.4f, 0.0f);
            light_node.set_local_transform(light_transform);
            auto light = std::make_unique<LightComponent>();
            light->type = LightType::DIRECTIONAL;
            light->color = {1.0f, 0.97f, 0.9f};
            light->intensity = 1.0f;
            light_node.add_component(std::move(light));

            std::cout << get_device_info() << '\n';
            set_swap_interval(app, SwapInterval::VSYNC);
        }

        bool on_event(const SDL_Event& event) override
        {
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                camera_->aspect = application().viewport().aspect_ratio();
                return true;
            }
            return false;
        }

        void on_update() override
        {
            const auto t = float(SDL_GetTicks() - start_ticks_) / 1000.0f;

            // Rotating the hub is all it takes to orbit the children: the
            // parent's world-version bump invalidates them (§2).
            Transform hub;
            hub.rotation = Xyz::Orientation3F(0.4f * t, 0.2f * t, 0.0f);
            hub_->set_local_transform(hub);

            spin_child(*left_, -2.2f, 1.7f * t);
            spin_child(*right_, 2.2f, -1.3f * t);
        }

        void on_draw() override
        {
            set_face_culling_enabled(true);
            set_clear_color({0.12f, 0.13f, 0.16f, 1.0f});
            clear(ClearBits::COLOR_DEPTH);
            const auto [width, height] = application().window_size();
            set_viewport(0, 0, width, height);

            resources_.begin_frame(frame_);

            TransformUpdater::resolve(scene_);
            auto& snapshots = scene_.snapshots();
            builder_.build(scene_, *camera_, snapshots.back());
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
                                  std::vector<uint8_t> params)
        {
            Material material;
            material.shader = shader;
            material.parameter_data = std::move(params);
            material.transparent = transparent;
            return resources_.create_material(std::move(material));
        }

        MeshRef make_cube_mesh()
        {
            static_assert(sizeof(PositionNormalTexture) == 32,
                          "the PNT vertex must match builtin_pnt_layout");

            MeshData<PositionNormalTexture> data;
            MeshDataBuilder builder(data);
            add_cube_pnt(builder);

            const auto layout = resources_.register_layout(
                builtin_pnt_layout());
            vbo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, 32,
                uint32_t(data.vertices.size()));
            ebo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, 2, uint32_t(data.indices.size()));

            const auto vertices = resources_.allocate(
                vbo_arena_, uint32_t(data.vertices.size()));
            const auto indices = resources_.allocate(
                ebo_arena_, uint32_t(data.indices.size()));

            // Rebase to absolute indices (§3): the portable path has no
            // baseVertex draw argument.
            for (auto& index : data.indices)
                index = uint16_t(index + vertices.offset);

            resources_.upload(vertices, data.vertices.data(),
                              data.vertices.size() * 32);
            resources_.upload(indices, data.indices.data(),
                              data.indices.size() * 2);

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

        void add_renderable(Node& node, MeshRef mesh, MaterialRef material)
        {
            auto renderable = std::make_unique<RenderableComponent>();
            renderable->mesh = mesh;
            renderable->material = material;
            renderable->local_bounds = unit_box();
            node.add_component(std::move(renderable));
        }

        Node& add_child_cube(MeshRef mesh, MaterialRef material, float x)
        {
            auto& child = hub_->add_child(std::make_unique<Node>());
            spin_child(child, x, 0);
            add_renderable(child, mesh, material);
            return child;
        }

        static void spin_child(Node& child, float x, float angle)
        {
            Transform transform;
            transform.translation = {x, 0, 0};
            transform.rotation = Xyz::Orientation3F(angle, 0.5f * angle, 0);
            transform.scale = {0.6f, 0.6f, 0.6f};
            child.set_local_transform(transform);
        }

        ResourceManager resources_;
        Scene scene_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        BufferArenaRef vbo_arena_;
        BufferArenaRef ebo_arena_;
        Node* hub_ = nullptr;
        Node* left_ = nullptr;
        Node* right_ = nullptr;
        CameraComponent* camera_ = nullptr;
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
