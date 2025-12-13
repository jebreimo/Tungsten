//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlApplication.hpp"

#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <Argos/ArgumentParser.hpp>
#include "Tungsten/EventLoop.hpp"
#include "Tungsten/GlContext.hpp"
#include "Tungsten/GlParameters.hpp"
#include "Tungsten/TungstenException.hpp"
#include "CommandLine.hpp"
#include "Tungsten/SdlDisplay.hpp"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

namespace Tungsten
{
    namespace
    {
#ifdef __EMSCRIPTEN__

        EM_JS(int, canvas_get_width, (),
        {
            return Module.canvas.width;
        });

        EM_JS(int, canvas_get_height, (),
        {
            return Module.canvas.height;
        });

        WindowParameters get_default_window_parameters()
        {
            int width = canvas_get_width();
            int height = canvas_get_height();

            WindowParameters params;
            if (width > 0 && height > 0)
                params.window_size = {width, height};
            return params;
        }

#else

        WindowParameters get_default_window_parameters()
        {
            return {};
        }

#endif
    }

    struct SdlApplication::Data
    {
        explicit Data(std::string name)
            : name(std::move(name))
        {
        }

        EventLoop* event_loop = nullptr;
        SdlSession* session = nullptr;

        std::string name;
        WindowParameters window_parameters;
        EventLoopMode event_loop_mode = EventLoopMode::UPDATE_CONTINUOUSLY;
        int status = 0;
        SDL_Window* window = nullptr;
        GlContext gl_context;
        bool enable_touch_events = false;
    };

    SdlApplication::SdlApplication() = default;

    SdlApplication::SdlApplication(std::string name)
        : data_(std::make_unique<Data>(std::move(name)))
    {
        data_->window_parameters = get_default_window_parameters();
    }

    SdlApplication::SdlApplication(SdlApplication&&) noexcept = default;

    SdlApplication::~SdlApplication() = default;

    SdlApplication& SdlApplication::operator=(SdlApplication&&) noexcept = default;

    const std::string& SdlApplication::name() const
    {
        return data_->name;
    }

    void SdlApplication::add_command_line_options(argos::ArgumentParser& parser)
    {
        Tungsten::add_command_line_options(parser);
    }

    void SdlApplication::read_command_line_options(const argos::ParsedArguments& args)
    {
        Tungsten::read_command_line_options(args, *this);
    }

    void SdlApplication::parse_command_line_options(int& argc, char**& argv)
    {
        Tungsten::parse_command_line_options(argc, argv, *this);
    }

    bool SdlApplication::is_running() const
    {
        return data_->event_loop != nullptr;
    }

    void SdlApplication::quit()
    {
        set_status(1);
    }

    SDL_GLContext SdlApplication::gl_context() const
    {
        return data_->gl_context;
    }

    int SdlApplication::status() const
    {
        return data_->status;
    }

    void SdlApplication::set_status(int status)
    {
        data_->status = status;
    }

    SDL_Window* SdlApplication::window() const
    {
        return data_->window;
    }

    void SdlApplication::set_window(SDL_Window* window)
    {
        data_->window = window;
    }

    Size2I SdlApplication::window_size() const
    {
        if (!window())
            TUNGSTEN_THROW("window is NULL!");

        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(window(), &w, &h);
        return {w, h};
    }

    void SdlApplication::process_event(const SDL_Event& event)
    {
        if (!data_->event_loop->on_event(event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                quit();
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_ESCAPE)
                    quit();
                break;
            default:
                break;
            }
        }
    }

    void SdlApplication::initialize(const WindowParameters& params)
    {
        set_sdl_gl_parameters(params.gl_parameters);

        auto tmpWindowParams = params;
        tmpWindowParams.sdl_flags |= SDL_WINDOW_OPENGL;

        set_window(create_window(tmpWindowParams));

        data_->gl_context = GlContext::create(window());

        glewExperimental = GL_TRUE;
        glewInit();
    }

    SDL_Window* SdlApplication::create_window(const WindowParameters& params)
    {
        SdlDisplayModesPtr display_modes;
        const SDL_DisplayMode* mode = nullptr;

        auto size = params.window_size;

        if (params.full_screen_mode)
        {
            display_modes = get_sdl_display_modes(params.full_screen_mode.display);
            mode = get_sdl_display_mode(display_modes.get(), params.full_screen_mode.mode);
            size = {mode->w, mode->h};
        }

        if (!size)
            size = {640, 480};

        SDL_Log("Create window with size %dx%d.", size.width, size.height);
        auto* window = SDL_CreateWindow(data_->name.c_str(),
                                        size.width, size.height,
                                        params.sdl_flags);
        if (window == nullptr)
            THROW_SDL_ERROR();

        if (params.full_screen_mode)
        {
            SDL_Log("Set fullscreen mode: %d:%d.",
                    params.full_screen_mode.display,
                    params.full_screen_mode.mode);
            if (!SDL_SetWindowFullscreenMode(window, mode)
                || !SDL_SetWindowFullscreen(window, true))
            {
                THROW_SDL_ERROR();
            }
        }

        return window;
    }

    void SdlApplication::run_event_loop(SdlSession& session, EventLoop& event_loop)
    {
        data_->session = &session;
        data_->event_loop = &event_loop;
        try
        {
            run_event_loop();
            data_->session = nullptr;
            data_->event_loop = nullptr;
        }
        catch (...)
        {
            data_->session = nullptr;
            data_->event_loop = nullptr;
            throw;
        }
    }

#ifdef __EMSCRIPTEN__

    void SdlApplication::run_event_loop()
    {
        const int simulate_infinite_loop = 1; // call the function repeatedly
        const int fps = -1; // call the function as fast as the browser wants to render (typically 60fps)
        emscripten_set_main_loop_arg(
            &SdlApplication::emscripten_event_loop_step,
            this, fps, simulate_infinite_loop);
    }

    void SdlApplication::emscripten_event_loop_step(void* arg)
    {
        try
        {
            auto app = static_cast<SdlApplication*>(arg);
            app->run_event_loop_step();
        }
        catch (const std::exception& ex)
        {
            SDL_Log("Exception: %s", ex.what());
            emscripten_cancel_main_loop();
        }
    }

#else

    void SdlApplication::run_event_loop()
    {
        while (!status())
            run_event_loop_step();
    }

#endif

    void SdlApplication::run_event_loop_step()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            process_event(event);

        data_->event_loop->on_update();

        if (data_->event_loop->should_redraw())
        {
            data_->event_loop->clear_redraw();
            data_->event_loop->on_draw();
            SDL_GL_SwapWindow(window());
        }

        if (!data_->event_loop->should_redraw()
            && data_->event_loop_mode == EventLoopMode::WAIT_FOR_EVENTS)
        {
            SDL_WaitEventTimeout(nullptr, 10);
        }
    }

    const EventLoop* SdlApplication::event_loop() const
    {
        return data_->event_loop;
    }

    EventLoop* SdlApplication::event_loop()
    {
        return data_->event_loop;
    }

    const WindowParameters& SdlApplication::window_parameters() const
    {
        return data_->window_parameters;
    }

    void SdlApplication::set_window_parameters(const WindowParameters& params)
    {
        data_->window_parameters = params;
    }

    EventLoopMode SdlApplication::event_loop_mode() const
    {
        return data_->event_loop_mode;
    }

    void SdlApplication::set_event_loop_mode(EventLoopMode mode)
    {
#ifdef __EMSCRIPTEN__
        if (mode == EventLoopMode::WAIT_FOR_EVENTS)
            return;
#endif
        data_->event_loop_mode = mode;
    }

    bool SdlApplication::touch_events_enabled() const
    {
        return data_->enable_touch_events;
    }

    void SdlApplication::set_touch_events_enabled(bool value)
    {
        data_->enable_touch_events = value;
        if (data_->session)
            data_->session->set_touch_events_enabled(value);
    }

    EventLoop& SdlApplication::callbacks()
    {
        return *data_->event_loop;
    }

    const EventLoop& SdlApplication::callbacks() const
    {
        return *data_->event_loop;
    }

    SdlSession SdlApplication::make_sdl_session()
    {
        constexpr uint32_t flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
        SdlSession session(flags);
        session.set_touch_events_enabled(data_->enable_touch_events);
        initialize(data_->window_parameters);
        return session;
    }

    float aspect_ratio(const SdlApplication& app)
    {
        auto [w, h] = app.window_size();
        return float(w) / float(h);
    }

    SwapInterval swap_interval(const SdlApplication&)
    {
        int result = 0;
        if (!SDL_GL_GetSwapInterval(&result))
            THROW_SDL_ERROR();
        return SwapInterval(result);
    }

    void set_swap_interval(const SdlApplication&, SwapInterval interval)
    {
        auto result = SDL_GL_SetSwapInterval(int(interval));
        if (!result && interval == SwapInterval::ADAPTIVE_VSYNC_OR_VSYNC)
            result = SDL_GL_SetSwapInterval(1);
        if (!result)
            THROW_SDL_ERROR();
    }

    void print_exception_recursive(const std::exception& e, int level) // NOLINT(*-no-recursion)
    {
        // Copied from https://cppreference.com/w/cpp/error/rethrow_if_nested.html
        std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
        try
        {
            std::rethrow_if_nested(e);
        }
        catch (const std::exception& nestedException)
        {
            print_exception_recursive(nestedException, level + 1);
        }
        catch (...)
        {
        }
    }

    void print_exception(const std::exception& e) // NOLINT(*-no-recursion)
    {
        print_exception_recursive(e, 0);
    }
}
