//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>
#include <Ystring/Ystring.hpp>

#include "CubeModel.hpp"

namespace
{
    class Cube : public Tungsten::EventLoop
    {
    public:
        explicit Cube(Tungsten::SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            Tungsten::register_builtin_shader_families(resources_);

            cube_ = add_cube_model(resources_, scene_);
            hub_ = {&scene_, cube_.root};

            camera_ = scene_.add_node();
            camera_.set_local_transform({
                .translation = {2, 2, 8},
                .rotation = Tungsten::look_at_rotation({2, 2, 8}, {0, 0, 0})
            });
            camera_.add(Tungsten::CameraComponent{
                .near_plane = 0.5f,
                .far_plane = 50.0f,
                .aspect = app.viewport().aspect_ratio()
            });

            auto light_node = scene_.add_node();
            // A directional light shines along its node's -z axis; tilt the
            // node so the light comes in from the upper right.
            light_node.set_local_transform({
                .rotation = Tungsten::euler_rotation(0.1f, 0.6f, 0.0f)
            });
            light_node.add(Tungsten::LightComponent{
                .type = Tungsten::LightType::DIRECTIONAL,
                .color = {1.0f, 0.97f, 0.9f},
                .intensity = 1.0f
            });
            Tungsten::set_swap_interval(app, Tungsten::SwapInterval::VSYNC);
        }

        void on_update() override
        {
            Tungsten::Transform hub;
            hub.rotation = get_rotation(SDL_GetTicks());
            hub_.set_local_transform(hub);
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            // Depth and culling are no longer set here: they travel on the
            // pipeline each material names.
            const Tungsten::RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.2f, 0.3f, 0.3f, 1.0f}}
            };
            camera_.get<Tungsten::CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.back().time =
                static_cast<float>(SDL_GetTicks() - start_ticks_) / 1000.0f;
            // The snapshot mirrors the per-frame UBO, so its colours are
            // linear; this one is picked by eye, hence sRGB.
            snapshots_.back().ambient_light =
                Tungsten::srgb_to_linear(Xyz::Vector3F{0.35f, 0.35f, 0.38f});
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        static Xyz::QuaternionF get_rotation(uint64_t ticks)
        {
            double i;
            const auto fraction = std::modf(double(ticks) / 5000, &i);
            const auto angle = float(fraction * 2 * Xyz::Constants<double>::PI);
            if ((ticks / 10000) % 2 == 0)
                return Tungsten::euler_rotation(0, angle, 0);
            return Tungsten::euler_rotation(0, 0, -angle);
        }

        Tungsten::ResourceManager resources_;
        Tungsten::Scene scene_;
        Tungsten::DoubleBuffer<Tungsten::RenderSnapshot> snapshots_;
        Tungsten::SnapshotBuilder builder_{resources_};
        Tungsten::Renderer renderer_;
        // Owns the cube's mesh, material and texture.
        Tungsten::Model cube_;
        Tungsten::NodeHandle hub_;
        Tungsten::NodeHandle camera_;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char** argv)
{
    try
    {
        Tungsten::SdlApplication app("TexturedCube");
        app.parse_command_line_options(argc, argv);
        app.run<Cube>();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
