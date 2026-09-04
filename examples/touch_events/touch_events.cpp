//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-12-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// A scrolling log of touch and wheel events, drawn through Neo's scene graph.
//
//     TextSystem::update(scene)        <- compiles text into meshes/materials
//     TextScroller::layout(viewport)   <- reads the bounds update() just filled
//     scene.resolve_transforms()
//     SnapshotBuilder::build(...)      <- culls the lines that scrolled off
//     Renderer::render(...)
//
// The layout pass sits between update() and the resolve because it needs to
// know how tall each line came out, and that is only known once the text has
// been tessellated. It then does nothing but write node transforms, so
// restacking the whole log re-uploads not one vertex.
//
// Lines that scroll off the top are simply left where they are. Their bounds
// put them outside the camera's frustum, so the snapshot's cull drops them
// before they ever reach a draw call — there is no visibility bookkeeping to
// do here.

#include <sstream>
#include <string>
#include <Tungsten/Tungsten.hpp>
#include "TextScroller.hpp"
#include "../show_text/Debug.hpp"

namespace
{
    class TouchEventsLoop : public Tungsten::EventLoop
    {
    public:
        explicit TouchEventsLoop(Tungsten::SdlApplication& app)
            : EventLoop(app),
              text_system_(resources_),
              renderer_(resources_),
              text_scroller_(scene_, font_manager_.default_font())
        {
            // A 2D scene is nodes at z = 0 seen by an orthographic camera. The
            // camera sits in front of that plane looking down its node's -z
            // axis.
            camera_ = scene_.add_node();
            camera_.set_local_transform(Tungsten::at(0, 0, 100));
            camera_.add(Tungsten::CameraComponent{
                .mode = Tungsten::ProjectionMode::ORTHOGRAPHIC,
                .near_plane = 1.0f,
                .far_plane = 200.0f,
                .aspect = app.viewport().aspect_ratio()
            });
        }

        void on_finger_event(const SDL_TouchFingerEvent& event)
        {
            JEB_CHECKPOINT();
            std::ostringstream ss;
            switch (event.type)
            {
            case SDL_EVENT_FINGER_DOWN: ss << "finger down: ";
                break;
            case SDL_EVENT_FINGER_UP: ss << "finger up: ";
                break;
            case SDL_EVENT_FINGER_MOTION: ss << "finger motion: ";
                break;
            case SDL_EVENT_FINGER_CANCELED: ss << "finger canceled: ";
                break;
            default: ss << "unknown: ";
                break;
            }
            ss << "id " << event.fingerID << " " << event.x << " " << event.y
                << " " << event.dx << " " << event.dy
                << " " << event.pressure;
            text_scroller_.add_text(ss.str());
            redraw();
        }

        void on_mouse_wheel(const SDL_MouseWheelEvent& event)
        {
            std::ostringstream ss;
            ss << "wheel: " << event.x << " " << event.y;
#if SDL_VERSION_ATLEAST(3, 2, 12)
            ss << " " << event.integer_x << " " << event.integer_y;
#endif
            text_scroller_.add_text(ss.str());
            redraw();
        }

        bool on_event(const SDL_Event& event) override
        {
            switch (event.type)
            {
            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_UP:
            case SDL_EVENT_FINGER_MOTION:
            case SDL_EVENT_FINGER_CANCELED:
                on_finger_event(event.tfinger);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                on_mouse_wheel(event.wheel);
                break;
            default:
                return false;
            }
            return true;
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            Tungsten::set_viewport(viewport);
            Tungsten::set_clear_color(0.4f, 0.6f, 0.8f, 1.0f);
            Tungsten::clear(Tungsten::ClearBits::COLOR_DEPTH);

            // One world unit per pixel: the orthographic half-height is half
            // the viewport's, so the visible volume is exactly the window in
            // pixels, with the origin at its centre.
            auto& camera = camera_.get<Tungsten::CameraComponent>();
            camera.ortho_size = viewport.size[1] / 2.0f;
            camera.aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            text_system_.update(scene_);
            text_scroller_.layout(viewport);
            scene_.resolve_transforms();

            auto& snapshots = snapshots_;
            builder_.build(scene_, camera_.id(), snapshots.back());
            snapshots.swap();
            renderer_.render(snapshots.front());

            // Single-threaded: the frame just drawn is complete.
            resources_.collect_garbage(frame_);
            ++frame_;
        }

    private:
        Tungsten::ResourceManager resources_;
        Tungsten::FontManager font_manager_;
        Tungsten::Scene scene_;
        Tungsten::DoubleBuffer<Tungsten::RenderSnapshot> snapshots_;
        Tungsten::TextSystem text_system_;
        Tungsten::SnapshotBuilder builder_{resources_};
        Tungsten::Renderer renderer_;
        Tungsten::TextScroller text_scroller_;

        Tungsten::NodeHandle camera_;
        uint64_t frame_ = 0;
    };
}

int main(int argc, char* argv[])
{
    try
    {
        SDL_SetHint(SDL_HINT_TRACKPAD_IS_TOUCH_ONLY, "1");
        Tungsten::SdlApplication app("touch_events");
        app.parse_command_line_options(argc, argv);
        app.set_event_loop_mode(Tungsten::EventLoopMode::WAIT_FOR_EVENTS);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<TouchEventsLoop>();
    }
    catch (std::exception& ex)
    {
        Tungsten::print_exception(ex);
        return 1;
    }
    return 0;
}
