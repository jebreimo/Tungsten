//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-06.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// Normal mapping in the builtin BlinnPhong family, as a side-by-side: two
// cubes of the same colour material, the left one with a normal map and the
// right one without. The only difference between their materials is the
// third texture and the strength passed to
// make_blinn_phong_material_params.
//
// The family needs no TANGENT attribute — the shader builds a tangent basis
// from screen-space derivatives — so both cubes share one mesh with the
// ordinary position / normal / texcoord layout.
//
// The pair rotates under a fixed directional light. That movement is the
// point: a normal map is barely visible in a still frame and unmistakable
// once the highlight sweeps across the studs.

#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include <Tungsten/Gl/IOglWrapper.hpp>

#include "BumpCubeModel.hpp"

namespace
{
    using namespace Tungsten;

    class BumpCube : public EventLoop
    {
    public:
        explicit BumpCube(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            std::cout << get_device_info() << '\n';
            register_builtin_shader_families(resources_);

            cubes_ = add_bump_cube_model(resources_, scene_);
            hub_ = {&scene_, cubes_.root};

            camera_ = scene_.add_node();
            camera_.set_local_transform({
                .translation = {0, 2, 8},
                .rotation = look_at_rotation({0, 2, 8}, {0, 0, 0})
            });
            camera_.add(CameraComponent{
                .near_plane = 0.5f,
                .far_plane = 50.0f
            });

            auto light_node = scene_.add_node();
            // A directional light shines along its node's -z axis; tilt the
            // node so the light comes in from the upper right.
            light_node.set_local_transform({
                .rotation = euler_rotation(-0.6f, 0.4f, 0.0f)
            });
            light_node.add(LightComponent{
                .type = LightType::DIRECTIONAL,
                .color = {1.0f, 0.97f, 0.9f},
                .intensity = 1.0f
            });

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        void on_update() override
        {
            const auto t = float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            // Rotating the hub orbits both cubes and turns each about its own
            // axis, which is what sweeps the highlight over the studs.
            hub_.set_local_transform({.rotation = euler_rotation(0.5f * t, 0.25f * t, 0)});
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.14f, 0.15f, 0.18f, 1.0f}}
            };
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots_.back().ambient_light =
                srgb_to_linear(Xyz::Vector3F{0.30f, 0.30f, 0.33f});
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);

            set_ogl_tracing_enabled(false);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        // Owns the cubes' mesh, materials and normal map.
        Model cubes_;
        NodeHandle hub_;
        NodeHandle camera_;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char** argv)
{
    try
    {
        Tungsten::SdlApplication app("BumpCube");
        app.parse_command_line_options(argc, argv);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<BumpCube>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
