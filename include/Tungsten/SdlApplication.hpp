//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iosfwd>
#include <memory>
#include <vector>
#include <GL/glew.h>
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
    class SdlApplication;

    class SdlApplication
    {
    public:
        SdlApplication(std::string name,
                       std::unique_ptr<EventLoop> event_loop);

        SdlApplication(SdlApplication&&) = default;

        virtual ~SdlApplication();

        SdlApplication& operator=(SdlApplication&&) = default;

        [[nodiscard]]
        const std::string& name() const;

        static void add_command_line_options(argos::ArgumentParser& parser);

        void read_command_line_options(const argos::ParsedArguments& args);

        void parse_command_line_options(int& argc, char**& argv);

        void run();

        [[nodiscard]]
        bool is_running() const;

        void quit();

        [[nodiscard]]
        SDL_GLContext gl_context() const;

        [[nodiscard]]
        int status() const;

        SdlApplication& set_swap_interval(int interval);

        [[nodiscard]]
        SDL_Window* window() const;

        void set_window(SDL_Window* window);

        [[nodiscard]]
        std::pair<int, int> window_size() const;

        [[nodiscard]]
        float aspect_ratio() const;

        [[nodiscard]]
        const WindowParameters& window_parameters() const;

        void set_window_parameters(const WindowParameters& params);

        [[nodiscard]]
        const EventLoop& callbacks() const;

        EventLoop& callbacks();
    protected:
        void set_status(int status);

        bool process_event(const SDL_Event& event);

        void initialize(const WindowParameters& params);

        SDL_Window* create_window(const WindowParameters& params);
    private:
        void event_loop();

        void event_loop_step();

        #ifdef __EMSCRIPTEN__
        static void emscripten_event_loop_step(void* arg);
        #endif

        std::string name_;
        std::unique_ptr<EventLoop> event_loop_;
        WindowParameters window_parameters_;
        SDL_Window* window_ = nullptr;
        GlContext gl_context_ = {};
        int status_ = 0;
        bool is_running_ = false;
    };
}
