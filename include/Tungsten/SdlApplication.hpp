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
#include "EventLoop.hpp"
#include "SdlSession.hpp"
#include "GlTypes.hpp"
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

    template <typename T>
    concept DerivedFromEventLoop = std::is_base_of_v<EventLoop, T>;

    class SdlApplication
    {
    public:
        SdlApplication();

        explicit SdlApplication(std::string name);

        SdlApplication(SdlApplication&&) noexcept;

        virtual ~SdlApplication();

        SdlApplication& operator=(SdlApplication&&) noexcept;

        [[nodiscard]] const std::string& name() const;

        /**
         * @brief Add the standard command line options to @a parser.
         *
         * Use this function if you want to add the standard Tungsten command
         * line options to a custom command line parser.
         */
        static void add_command_line_options(argos::ArgumentParser& parser);

        /**
         * @brief Read the standard command line options from @a args.
         *
         * Use this function if you want to use the standard Tungsten command
         * line options with a custom command line parser.
         */
        void read_command_line_options(const argos::ParsedArguments& args);

        /**
         * @brief Parse the standard command line options from @a argc and
         *     @a argv.
         */
        void parse_command_line_options(int& argc, char**& argv);

        template <DerivedFromEventLoop EventLoopT, typename ...Args>
        void run(Args&&... args)
        {
            SdlSession session = make_sdl_session();
            EventLoopT event_loop(*this, std::forward<Args>(args)...);
            run_event_loop(session, event_loop);
        }

        [[nodiscard]] bool is_running() const;

        void quit();

        [[nodiscard]] SDL_GLContext gl_context() const;

        [[nodiscard]] int status() const;

        [[nodiscard]] SDL_Window* window() const;

        void set_window(SDL_Window* window);

        [[nodiscard]] Size2I window_size() const;

        [[nodiscard]] const EventLoop* event_loop() const;

        [[nodiscard]] EventLoop* event_loop();

        [[nodiscard]] const WindowParameters& window_parameters() const;

        void set_window_parameters(const WindowParameters& params);

        [[nodiscard]] EventLoopMode event_loop_mode() const;

        void set_event_loop_mode(EventLoopMode mode);

        [[nodiscard]] bool touch_events_enabled() const;

        void set_touch_events_enabled(bool value);

        [[nodiscard]] const EventLoop& callbacks() const;

        [[nodiscard]] EventLoop& callbacks();
    protected:
        void set_status(int status);

        void process_event(const SDL_Event& event);

        void initialize(const WindowParameters& params);

        SDL_Window* create_window(const WindowParameters& params);
    private:
        void run_event_loop(SdlSession& session, EventLoop& event_loop);

        void run_event_loop();

        void run_event_loop_step();

        [[nodiscard]] SdlSession make_sdl_session();

        #ifdef __EMSCRIPTEN__
        static void emscripten_event_loop_step(void* arg);
        #endif

        struct Data;
        std::unique_ptr<Data> data_;
    };

    [[nodiscard]] float aspect_ratio(const SdlApplication& app);

    [[nodiscard]] SwapInterval swap_interval(const SdlApplication& app);

    void set_swap_interval(const SdlApplication& app, SwapInterval interval);

    /**
     * @brief Prints the given exception to the standard error stream.
     *
     * This is a convenience function that prints nested exceptions.
     *
     * @param e an exception to print
     */
    void print_exception(const std::exception& e);
}
