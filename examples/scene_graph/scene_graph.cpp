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
#include <Tungsten/Gl/IOglWrapper.hpp>

#include "cube_hub_model.hpp"

namespace
{
    using namespace Tungsten;

    class SceneGraphLoop : public EventLoop
    {
    public:
        explicit SceneGraphLoop(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            register_builtin_shader_families(resources_);

            auto cubes = add_cube_hub_model(resources_, scene_);
            cubes_ = std::move(cubes.model);
            hub_ = {&scene_, cubes_.root};
            left_ = {&scene_, cubes.left};
            right_ = {&scene_, cubes.right};

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

            spin_child(left_, -CHILD_CUBE_OFFSET, 1.7f * t);
            spin_child(right_, CHILD_CUBE_OFFSET, -1.3f * t);

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
            const auto viewport = application().viewport();
            // Where to draw and what to do with the surface around the draw.
            // Culling is no longer set here: it travels on each pipeline.
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.12f, 0.13f, 0.16f, 1.0f}}
            };
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            auto& snapshots = snapshots_;
            builder_.build(scene_, camera_.id(), snapshots.back());
            snapshots.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots.back().ambient_light =
                Tungsten::srgb_to_linear(Xyz::Vector3F{0.35f, 0.35f, 0.38f});
            snapshots.swap();

            renderer_.render(snapshots.front(), pass);

            // Single-threaded: the frame just drawn is complete (§11).
            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
            set_ogl_tracing_enabled(false);
        }

    private:
        static void spin_child(NodeHandle child, float x, float angle)
        {
            Transform transform;
            transform.translation = {x, 0, 0};
            transform.rotation = euler_rotation(angle, 0.5f * angle, 0);
            transform.scale = {CHILD_CUBE_SCALE, CHILD_CUBE_SCALE, CHILD_CUBE_SCALE};
            child.set_local_transform(transform);
        }

        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        // Owns the cubes' mesh and materials.
        Model cubes_;
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
