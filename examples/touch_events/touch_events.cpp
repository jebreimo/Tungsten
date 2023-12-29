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

#include "Debug.hpp"

class EventLoop : public Tungsten::EventLoop
{
public:
    EventLoop()
        : text_renderer_(Tungsten::FontManager::instance().default_font())
    {}

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        std::string msg;
        switch (event.type)
        {
        case SDL_FINGERDOWN:
            msg = "SDL_FINGERDOWN";
            break;
        case SDL_FINGERUP:
            msg = "SDL_FINGERUP";
            break;
        case SDL_FINGERMOTION:
            msg = "SDL_FINGERMOTION";
            break;
        case SDL_MULTIGESTURE:
            msg = "SDL_MULTIGESTURE";
            break;
        case SDL_MOUSEBUTTONDOWN:
            msg = "SDL_MOUSEBUTTONDOWN";
            break;
        case SDL_MOUSEBUTTONUP:
            msg = "SDL_MOUSEBUTTONUP";
            break;
        default:
            return false;
        }
        if (!msg.empty())
        {
            texts_.push(Yconvert::convert_to<std::u32string>(
                msg,
                Yconvert::Encoding::UTF_8,
                Yconvert::Encoding::UTF_32_NATIVE));
            SDL_Log("%s", msg.c_str());
            redraw();
        }
        return true;
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
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("ShowText",
                                     std::make_unique<EventLoop>());
        app.parse_command_line_options(argc, argv);
        app.set_event_loop_mode(Tungsten::EventLoopMode::WAIT_FOR_EVENTS);
        app.run({.enable_touch_events = true});
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
