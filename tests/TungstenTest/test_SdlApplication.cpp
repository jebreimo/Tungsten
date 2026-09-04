//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Sdl/SdlApplication.hpp"

#include <functional>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Sdl/EventLoop.hpp"

using namespace Tungsten;

namespace
{
    // Records the order the stack visited it in, and can be told to consume
    // events or to edit the stack from inside a callback.
    class RecordingLoop : public EventLoop
    {
    public:
        RecordingLoop(SdlApplication& app, std::string name,
                      std::vector<std::string>& log)
            : EventLoop(app), name_(std::move(name)), log_(&log)
        {}

        bool on_event(const SDL_Event&) override
        {
            log_->push_back(name_);
            if (on_event_action)
                on_event_action();
            return consumes;
        }

        bool consumes = false;
        std::function<void()> on_event_action;

    private:
        std::string name_;
        std::vector<std::string>* log_;
    };

    // process_event is protected: drive it from a subclass rather than
    // standing up a window and a real run loop.
    class TestApplication : public SdlApplication
    {
    public:
        TestApplication() : SdlApplication("test") {}

        using SdlApplication::process_event;
    };

    SDL_Event make_key_event()
    {
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_UP;
        event.key.key = SDLK_A;
        return event;
    }
}

TEST_CASE("SdlApplication: pushing outside a traversal takes effect at once")
{
    TestApplication app;
    std::vector<std::string> log;
    RecordingLoop bottom(app, "bottom", log);
    RecordingLoop top(app, "top", log);

    REQUIRE(app.event_loops().empty());
    REQUIRE(app.event_loop() == nullptr);
    REQUIRE_FALSE(app.is_running());

    app.push_event_loop(bottom);
    app.push_event_loop(top);
    REQUIRE(app.event_loops().size() == 2);
    // Bottom first, and event_loop() names the bottom of the stack.
    REQUIRE(app.event_loops()[0] == &bottom);
    REQUIRE(app.event_loops()[1] == &top);
    REQUIRE(app.event_loop() == &bottom);

    // Pushing the same loop again does not stack it twice.
    app.push_event_loop(top);
    REQUIRE(app.event_loops().size() == 2);

    app.remove_event_loop(bottom);
    REQUIRE(app.event_loops().size() == 1);
    REQUIRE(app.event_loops()[0] == &top);

    // Removing something that is not on the stack is harmless.
    app.remove_event_loop(bottom);
    REQUIRE(app.event_loops().size() == 1);
}

TEST_CASE("SdlApplication: events go to the top loop first")
{
    TestApplication app;
    std::vector<std::string> log;
    RecordingLoop bottom(app, "bottom", log);
    RecordingLoop middle(app, "middle", log);
    RecordingLoop top(app, "top", log);
    app.push_event_loop(bottom);
    app.push_event_loop(middle);
    app.push_event_loop(top);

    app.process_event(make_key_event());

    // Top down — the reverse of the draw order.
    REQUIRE(log == std::vector<std::string>{"top", "middle", "bottom"});
}

TEST_CASE("SdlApplication: a loop that handles an event consumes it")
{
    TestApplication app;
    std::vector<std::string> log;
    RecordingLoop bottom(app, "bottom", log);
    RecordingLoop top(app, "top", log);
    app.push_event_loop(bottom);
    app.push_event_loop(top);

    top.consumes = true;
    app.process_event(make_key_event());

    // The scene beneath never sees the click the overlay took.
    REQUIRE(log == std::vector<std::string>{"top"});
}

TEST_CASE("SdlApplication: an unclaimed escape still quits")
{
    TestApplication app;
    std::vector<std::string> log;
    RecordingLoop loop(app, "loop", log);
    app.push_event_loop(loop);

    SDL_Event event{};
    event.type = SDL_EVENT_KEY_UP;
    event.key.key = SDLK_ESCAPE;

    REQUIRE(app.status() == 0);
    app.process_event(event);
    REQUIRE(app.status() != 0);
    REQUIRE(log == std::vector<std::string>{"loop"});
}

TEST_CASE("SdlApplication: a loop consuming escape keeps the app running")
{
    TestApplication app;
    std::vector<std::string> log;
    RecordingLoop loop(app, "loop", log);
    loop.consumes = true;
    app.push_event_loop(loop);

    SDL_Event event{};
    event.type = SDL_EVENT_KEY_UP;
    event.key.key = SDLK_ESCAPE;

    app.process_event(event);
    REQUIRE(app.status() == 0);
}

TEST_CASE("SdlApplication: a loop may edit the stack from inside a callback")
{
    TestApplication app;
    std::vector<std::string> log;
    RecordingLoop bottom(app, "bottom", log);
    RecordingLoop top(app, "top", log);
    RecordingLoop added(app, "added", log);
    app.push_event_loop(bottom);
    app.push_event_loop(top);

    // Editing the stack mid-traversal must not invalidate the walk in
    // progress: both existing loops still see this event.
    top.on_event_action = [&]
    {
        app.push_event_loop(added);
        app.remove_event_loop(top);
    };

    app.process_event(make_key_event());
    REQUIRE(log == std::vector<std::string>{"top", "bottom"});

    // process_event is not itself a phase, so the edits apply immediately
    // here; inside a real run they would apply at the phase boundary.
    REQUIRE(app.event_loops().size() == 2);
    REQUIRE(app.event_loops()[0] == &bottom);
    REQUIRE(app.event_loops()[1] == &added);
}
