//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-06.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// Tungsten's InfiniteGrid: an infinite grid on the world's xz plane, with the
// x axis in red, the z axis in blue and the y axis in green.
//
// Controls: drag with the left mouse button to orbit, with the right button
// to move across the grid, and use the wheel to zoom.

#include <iostream>

#include "Tungsten/Tungsten.hpp"

namespace
{
    using namespace Tungsten;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;

    class Coordsys : public EventLoop
    {
    public:
        explicit Coordsys(SdlApplication& app)
            : EventLoop(app),
              grid_{resources_},
              renderer_(resources_)
        {
            grid_.add_to(scene_);
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
                    navigator_.orbit(event.motion.xrel, event.motion.yrel);
                else if (event.motion.state & SDL_BUTTON_RMASK)
                    navigator_.pan(event.motion.xrel, event.motion.yrel);
                else
                    return false;
                place_camera();
                return true;
            case SDL_EVENT_MOUSE_WHEEL:
                navigator_.zoom(event.wheel.y);
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
        // Places the camera on a sphere around the target, looking at it.
        void place_camera()
        {
            const auto position = navigator_.position();
            camera_.set_local_transform({
                .translation = position,
                .rotation = look_at_rotation(position, navigator_.target)
            });
        }

        ResourceManager resources_;
        InfiniteGrid grid_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        NodeHandle camera_;
        OrbitNavigator navigator_;
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
