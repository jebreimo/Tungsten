//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iosfwd>
#include "EventLoop.hpp"
#include "GlContext.hpp"
#include "WindowParameters.hpp"

namespace argos
{
    class ArgumentParser;
    class ParsedArguments;
}

namespace Tungsten
{
    enum class EventLoopMode
    {
        UPDATE_CONTINUOUSLY,
        WAIT_FOR_EVENTS
    };

    enum class SwapInterval
    {
        IMMEDIATE = 0,
        VSYNC = 1,
        ADAPTIVE_VSYNC = -1,
        ADAPTIVE_VSYNC_OR_VSYNC = -2
    };

    struct SdlConfiguration
    {
        bool enable_touch_events = false;
        bool enable_timers = false;
    };

    class SdlApplication
    {
    public:
        SdlApplication();

        SdlApplication(std::string name,
                       std::unique_ptr<EventLoop> event_loop);

        SdlApplication(SdlApplication&&) noexcept;

        virtual ~SdlApplication();

        SdlApplication& operator=(SdlApplication&&) noexcept;

        [[nodiscard]] const std::string& name() const;

        static void add_command_line_options(argos::ArgumentParser& parser);

        void read_command_line_options(const argos::ParsedArguments& args);

        void parse_command_line_options(int& argc, char**& argv);

        void run(const SdlConfiguration& configuration = {});

        [[nodiscard]] bool is_running() const;

        void quit();

        [[nodiscard]] SDL_GLContext gl_context() const;

        [[nodiscard]] int status() const;

        [[nodiscard]] SDL_Window* window() const;

        void set_window(SDL_Window* window);

        [[nodiscard]] std::pair<int, int> window_size() const;

        [[nodiscard]] const EventLoop* event_loop() const;

        [[nodiscard]] EventLoop* event_loop();

        [[nodiscard]] const WindowParameters& window_parameters() const;

        void set_window_parameters(const WindowParameters& params);

        [[nodiscard]] EventLoopMode event_loop_mode() const;

        void set_event_loop_mode(EventLoopMode mode);

        [[nodiscard]]
        const EventLoop& callbacks() const;

        [[nodiscard]] EventLoop& callbacks();
    protected:
        void set_status(int status);

        void process_event(const SDL_Event& event);

        void initialize(const WindowParameters& params);

        SDL_Window* create_window(const WindowParameters& params);
    private:
        void run_event_loop();

        void run_event_loop_step();

        #ifdef __EMSCRIPTEN__
        static void emscripten_event_loop_step(void* arg);
        #endif

        struct Data;
        std::unique_ptr<Data> data_;
    };
}
