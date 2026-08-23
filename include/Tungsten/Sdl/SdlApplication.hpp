//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <span>
#include "EventLoop.hpp"
#include "SdlSession.hpp"
#include "../Viewport.hpp"
#include "../Gl/GlTypes.hpp"
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

        virtual ~SdlApplication();

        SdlApplication(SdlApplication&&) noexcept;

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

        [[nodiscard]] float aspect_ratio() const;

        [[nodiscard]] Viewport viewport() const;

        /**
         * @brief Adds an event loop on top of the stack.
         *
         * The application holds a stack of event loops rather than a single
         * one, so a scene, a text overlay and a set of UI controls can each be
         * their own loop over one window. The stack runs bottom to top:
         *
         * - **Events** go to the *top* loop first and stop at the first one
         *   whose on_event returns true. An overlay that handles a click
         *   therefore keeps it from reaching the scene beneath it.
         * - **on_update and on_draw** run bottom to top, so upper loops draw
         *   over lower ones.
         * - **Redrawing is all-or-nothing**: one loop asking for a redraw
         *   redraws them all, because they share one framebuffer and one
         *   buffer swap.
         *
         * The bottom loop is normally the one run() constructed. Because the
         * loops share a framebuffer, clearing is the bottom loop's job — an
         * upper loop that clears the colour buffer erases what is beneath it.
         *
         * The application does not take ownership: @a event_loop must outlive
         * the run, or be removed before it is destroyed.
         *
         * Pushing the same loop twice is a no-op. A push made from inside a
         * callback takes effect at the end of that phase — so a loop may add
         * or remove loops, including itself, without disturbing the traversal
         * in progress; a push made outside one, such as while setting the
         * application up before run(), takes effect immediately.
         */
        void push_event_loop(EventLoop& event_loop);

        /**
         * @brief Removes an event loop from the stack.
         *
         * Deferred to the end of the phase when called from inside a callback,
         * immediate otherwise. Removing a loop that is not on the stack does
         * nothing.
         */
        void remove_event_loop(EventLoop& event_loop);

        /**
         * @brief The event loops, bottom first.
         */
        [[nodiscard]] std::span<EventLoop* const> event_loops() const;

        /**
         * @brief The bottom loop of the stack, or nullptr if there is none.
         *
         * This is the loop run() constructed.
         */
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

        [[nodiscard]] double seconds_per_frame() const;
    protected:
        void set_status(int status);

        void process_event(const SDL_Event& event);

        void initialize(const WindowParameters& params);

        SDL_Window* create_window(const WindowParameters& params);
    private:
        void run_event_loop(SdlSession& session, EventLoop& event_loop);

        void run_event_loop();

        void run_event_loop_step();

        /**
         * Adds or removes an event loop, queueing the change if a phase is
         * currently walking the stack.
         */
        void modify_event_loops(EventLoop* event_loop, bool add);

        /**
         * Applies pushes and removals requested during a traversal. Called at
         * each phase boundary, so no phase ever iterates a stack that is being
         * modified underneath it.
         */
        void apply_pending_event_loops();

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
