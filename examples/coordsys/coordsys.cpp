//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-06.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// An infinite grid on the world's xz plane, with the x axis in red and the z
// axis in blue, drawn with the "pristine grid" shader from Ben Golus' article
// The Best Darn Grid Shader (Yet). A green line going up from the origin is
// the y axis.
//
// The grid is a single square that the vertex shader keeps centred below the
// camera, and which is big enough to reach past the far plane in every
// direction. All the work is in grid-frag.glsl. The y axis is another square,
// which axis-vert.glsl makes long and thin and turns towards the camera.
//
// Controls: drag with the left mouse button to orbit, with the right button
// to move across the grid, and use the wheel to zoom.

#include <algorithm>
#include <cmath>
#include <iostream>

#include "CoordinateSystem.hpp"
#include "Tungsten/Tungsten.hpp"
#include "Resources.hpp"

namespace
{
    using namespace Tungsten;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;

    constexpr float ORBIT_SPEED = 0.005f;
    constexpr float PAN_SPEED = 0.002f;
    constexpr float ZOOM_STEP = 1.1f;
    constexpr float MIN_DISTANCE = 0.5f;
    constexpr float MAX_DISTANCE = 500.0f;
    constexpr float MAX_PITCH = 1.55f;

    class Coordsys : public EventLoop
    {
    public:
        explicit Coordsys(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_),
              coordinate_system_{resources_, scene_}
        {
            camera_ = scene_.add_node();
            camera_.add(CameraComponent{
                .near_plane = NEAR_PLANE,
                .far_plane = FAR_PLANE,
                .aspect = app.viewport().aspect_ratio()
            });
            place_camera();

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        bool on_event(const SDL_Event& event) override
        {
            switch (event.type)
            {
            case SDL_EVENT_MOUSE_MOTION:
                if (event.motion.state & SDL_BUTTON_LMASK)
                    orbit(event.motion.xrel, event.motion.yrel);
                else if (event.motion.state & SDL_BUTTON_RMASK)
                    pan(event.motion.xrel, event.motion.yrel);
                else
                    return false;
                return true;
            case SDL_EVENT_MOUSE_WHEEL:
                distance_ = std::clamp(
                    distance_ * std::pow(ZOOM_STEP, -event.wheel.y),
                    MIN_DISTANCE, MAX_DISTANCE);
                place_camera();
                return true;
            default:
                return false;
            }
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.45f, 0.55f, 0.65f, 1.0f}}
            };
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        void orbit(float dx, float dy)
        {
            yaw_ -= dx * ORBIT_SPEED;
            pitch_ = std::clamp(pitch_ + dy * ORBIT_SPEED,
                                -MAX_PITCH, MAX_PITCH);
            place_camera();
        }

        // Moves the target across the grid such that the grid appears to
        // follow the mouse.
        void pan(float dx, float dy)
        {
            const auto step = PAN_SPEED * distance_;
            const Xyz::Vector3F right = {std::cos(yaw_), 0, -std::sin(yaw_)};
            const Xyz::Vector3F forward = {-std::sin(yaw_), 0, -std::cos(yaw_)};
            target_ = target_ - right * (dx * step) + forward * (dy * step);
            place_camera();
        }

        // Places the camera on a sphere around the target, looking at it.
        void place_camera()
        {
            const Xyz::Vector3F offset = {
                std::cos(pitch_) * std::sin(yaw_),
                std::sin(pitch_),
                std::cos(pitch_) * std::cos(yaw_)
            };
            const auto position = target_ + offset * distance_;
            camera_.set_local_transform({
                .translation = position,
                .rotation = look_at_rotation(position, target_)
            });
        }

        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        CoordinateSystem coordinate_system_;
        Renderer renderer_;
        NodeHandle camera_;
        Xyz::Vector3F target_ = {0, 0, 0};
        float yaw_ = 0.5f;
        float pitch_ = 0.35f;
        float distance_ = 8.0f;
        uint64_t frame_ = 0;
    };
}

int main(int argc, char* argv[])
{
    try
    {
        SdlApplication app("Coordsys");
        app.parse_command_line_options(argc, argv);
        app.run<Coordsys>();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
