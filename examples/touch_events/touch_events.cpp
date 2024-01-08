//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-12-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <thread>
#include <Tungsten/Tungsten.hpp>
#include <Yconvert/Convert.hpp>
#include "MouseWheelEventThrottler.hpp"
#include "MultiGestureEventThrottler.hpp"
#include "RingBuffer.hpp"

namespace
{
    std::u32string u8_to_u32(std::string_view str)
    {
        return Yconvert::convert_to<std::u32string>(
            str,
            Yconvert::Encoding::UTF_8,
            Yconvert::Encoding::UTF_32_NATIVE);
    }
}

class EventLoop : public Tungsten::EventLoop
{
public:
    EventLoop()
        : text_renderer_(Tungsten::FontManager::instance().default_font()),
          multi_gesture_throttler_(250)
    {}

    void on_multi_gesture(Tungsten::SdlApplication& app,
                          const SDL_MultiGestureEvent& event)
    {
        if (event.numFingers == 2)
        {
            std::ostringstream ss;
            if (event.dDist > 0.1)
                ss << "zoom in: " << event.dDist << " " << event.x << " " << event.y;
            else if (event.dDist < -0.1)
                ss << "zoom out: " << event.dDist << " " << event.x << " " << event.y;
            if (auto str = ss.str(); !str.empty())
                texts_.push(u8_to_u32(str));
        }
        redraw();
    }

    void on_mouse_wheel(Tungsten::SdlApplication& app,
                        const SDL_MouseWheelEvent& event)
    {
        std::ostringstream ss;
        ss << "wheel: " << event.preciseX << " " << event.preciseY;
        texts_.push(u8_to_u32(ss.str()));
        redraw();
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        has_event_ = true;
        std::string msg;
        switch (event.type)
        {
        //case SDL_FINGERDOWN:
        //    msg = "SDL_FINGERDOWN";
        //    break;
        //case SDL_FINGERUP:
        //    msg = "SDL_FINGERUP";
        //    break;
        //case SDL_FINGERMOTION:
        //    msg = "SDL_FINGERMOTION";
        //    break;
        case SDL_MULTIGESTURE:
            if (!multi_gesture_throttler_.update(event.mgesture))
            {
                on_multi_gesture(app, multi_gesture_throttler_.event());
                multi_gesture_throttler_.clear();
                multi_gesture_throttler_.update(event.mgesture);
            }
            else if (multi_gesture_throttler_.has_event(event.mgesture.timestamp))
            {
                on_multi_gesture(app, multi_gesture_throttler_.event());
                multi_gesture_throttler_.clear();
            }
           break;
        case SDL_MOUSEWHEEL:
            if (!mouse_wheel_throttler_.update(event.wheel))
            {
                on_mouse_wheel(app, mouse_wheel_throttler_.event());
                mouse_wheel_throttler_.clear();
                mouse_wheel_throttler_.update(event.wheel);
            }
            else if (mouse_wheel_throttler_.has_event(event.wheel.timestamp))
            {
                on_mouse_wheel(app, mouse_wheel_throttler_.event());
                mouse_wheel_throttler_.clear();
            }
            break;
        //case SDL_MOUSEBUTTONDOWN:
        //    msg = "SDL_MOUSEBUTTONDOWN";
        //    break;
        //case SDL_MOUSEBUTTONUP:
        //    msg = "SDL_MOUSEBUTTONUP";
        //    break;
        default:
            return false;
        }
        //if (!msg.empty())
        //{
        //    texts_.push(u8_to_u32(msg));
        //    SDL_Log("%s", msg.c_str());
        //    redraw();
        //}
        return true;
    }

    void on_update(Tungsten::SdlApplication& app) override
    {
        if (multi_gesture_throttler_.has_event(SDL_GetTicks()))
        {
            on_multi_gesture(app, multi_gesture_throttler_.event());
            multi_gesture_throttler_.clear();
            has_event_ = true;
        }
        if (mouse_wheel_throttler_.has_event(SDL_GetTicks()))
        {
            on_mouse_wheel(app, mouse_wheel_throttler_.event());
            mouse_wheel_throttler_.clear();
            has_event_ = true;
        }

        if (!has_event_)
        {
            #ifndef __EMSCRIPTEN__
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            #endif
        }
        has_event_ = false;
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        glClearColor(0.4, 0.6, 0.8, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto screen_size = Xyz::Vector2F(app.window_size());

        for (size_t i = 0; i < texts_.size(); ++i)
        {
            auto text = texts_[i];
            auto size = Tungsten::get_size(text, text_renderer_.font()) * 2.f
                        / screen_size;
            text_renderer_.draw(text,
                                {-1.0f, 1.0f - size[1] * float(i + 1)},
                                screen_size,
                                {.color = {0xFF, 0xFF, 0xFF, 0xFF}});
        }
    }

private:
    Tungsten::TextRenderer text_renderer_;
    Chorasmia::RingBuffer<std::u32string, 10> texts_;
    MouseWheelEventThrottler mouse_wheel_throttler_;
    MultiGestureEventThrottler multi_gesture_throttler_;
    bool has_event_ = false;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("ShowText",
                                     std::make_unique<EventLoop>());
        app.parse_command_line_options(argc, argv);
        app.run({.enable_touch_events = true});
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
