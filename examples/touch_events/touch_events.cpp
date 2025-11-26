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
#include "RingBuffer.hpp"
#include "../show_text/Debug.hpp"

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
    explicit EventLoop(Tungsten::SdlApplication& app)
        : Tungsten::EventLoop(app),
          text_renderer_(Tungsten::FontManager::instance().default_font())
    {
        set_swap_interval(application(), Tungsten::SwapInterval::ADAPTIVE_VSYNC_OR_VSYNC);
    }

    void on_finger_event(const SDL_TouchFingerEvent& event)
    {
        JEB_CHECKPOINT();
        std::ostringstream ss;
        switch (event.type)
        {
        case SDL_EVENT_FINGER_DOWN: ss << "finger down: "; break;
        case SDL_EVENT_FINGER_UP: ss << "finger up: "; break;
        case SDL_EVENT_FINGER_MOTION: ss << "finger motion: "; break;
        case SDL_EVENT_FINGER_CANCELED: ss << "finger canceled: "; break;
        default: ss << "unknown: "; break;
        }
        ss << "id " << event.fingerID << " " << event.x << " " << event.y
            << " " << event.dx << " " << event.dy
            << " " << event.pressure;
        if (auto str = ss.str(); !str.empty())
            texts_.push_back(u8_to_u32(str));
        redraw();
    }

    void on_mouse_wheel(const SDL_MouseWheelEvent& event)
    {
        JEB_CHECKPOINT();
        std::ostringstream ss;
        ss << "wheel: " << event.x << " " << event.y;
#if SDL_VERSION_ATLEAST(3, 2, 12)
        ss << " " << event.integer_x << " " << event.integer_y;
#endif
        texts_.push_back(u8_to_u32(ss.str()));
        redraw();
    }

    bool on_event(const SDL_Event& event) override
    {
        std::string msg;
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
        JEB_CHECKPOINT();
        glClearColor(0.4, 0.6, 0.8, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto screen_size = vector_cast<float>(application().window_size());

        auto line_height = text_renderer_.font().max_glyph.size[1];
        auto lines = std::min(texts_.size(), size_t(screen_size[1] / line_height));
        int i = 0;
        for (auto it = texts_.end() - lines; it != texts_.end(); ++it)
        {
            auto& text = *it;
            auto size = Tungsten::get_size(text, text_renderer_.font()) * 2.f
                        / screen_size;
            text_renderer_.draw(text,
                                {-1.0f, 1.0f - size[1] * float(++i)},
                                screen_size,
                                {.color = {0xFF, 0xFF, 0xFF, 0xFF}});
        }
    }

private:
    Tungsten::TextRenderer text_renderer_;
    Chorasmia::RingBuffer<std::u32string, 100> texts_;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("touch_events");
        app.parse_command_line_options(argc, argv);
        app.set_event_loop_mode(Tungsten::EventLoopMode::WAIT_FOR_EVENTS);
        app.run<EventLoop>();
    }
    catch (std::exception& ex)
    {
        Tungsten::print_exception(ex);
        return 1;
    }
    return 0;
}
