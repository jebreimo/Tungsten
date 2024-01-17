//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlApplication.hpp"

#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <Argos/ArgumentParser.hpp>
#include "Tungsten/EventLoop.hpp"
#include "Tungsten/GlParameters.hpp"
#include "Tungsten/TungstenException.hpp"
#include "CommandLine.hpp"
#include "SdlSession.hpp"

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

        void apply_configuration(const SdlConfiguration& configuration)
        {
            if (configuration.enable_touch_events)
                SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
        }

        uint32_t get_sdl_init_flags(const SdlConfiguration& configuration)
        {
            uint32_t flags = SDL_INIT_VIDEO;
            if (configuration.enable_timers)
                flags |= SDL_INIT_TIMER;
            return flags;
        }
    }

    struct SdlApplication::Data
    {
        Data(std::string name,
             std::unique_ptr<EventLoop> event_loop)
            : name(std::move(name)),
              event_loop(std::move(event_loop))
        {
        }

        std::string name;
        std::unique_ptr<EventLoop> event_loop;
        WindowParameters window_parameters;
        EventLoopMode event_loop_mode = EventLoopMode::UPDATE_CONTINUOUSLY;
        bool is_running = false;
        int status = 0;
        SDL_Window* window = nullptr;
        GlContext gl_context;
    };

    SdlApplication::SdlApplication() = default;

    SdlApplication::SdlApplication(
        std::string name,
        std::unique_ptr<EventLoop> event_loop)
        : data_(std::make_unique<Data>(std::move(name), std::move(event_loop)))
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

    void SdlApplication::run(const SdlConfiguration& configuration)
    {
        if (!data_->event_loop)
            TUNGSTEN_THROW("No eventloop.");
        apply_configuration(configuration);
        SdlSession session(get_sdl_init_flags(configuration));
        initialize(data_->window_parameters);
        data_->event_loop->on_startup(*this);
        data_->is_running = true;
        run_event_loop();
        data_->is_running = false;
        data_->event_loop->on_shutdown(*this);
    }

    bool SdlApplication::is_running() const
    {
        return data_->is_running;
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

    std::pair<int, int> SdlApplication::window_size() const
    {
        if (!window())
            TUNGSTEN_THROW("window is NULL!");

        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window(), &w, &h);
        return {w, h};
    }

    void SdlApplication::process_event(const SDL_Event& event)
    {
        if (!data_->event_loop->on_event(*this, event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit();
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit();
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    glViewport(0, 0, event.window.data1, event.window.data2);
                    data_->event_loop->redraw();
                }
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

    SDL_Window*
    SdlApplication::create_window(const WindowParameters& params)
    {
        const auto& pos = params.window_pos;
        auto size = params.window_size;
        if (params.full_screen_mode)
        {
            SDL_DisplayMode modeInfo = {};
            if (SDL_GetDisplayMode(params.full_screen_mode.display,
                                   params.full_screen_mode.mode,
                                   &modeInfo) >= 0)
            {
                size = {modeInfo.w, modeInfo.h};
            }
        }

        if (!size)
            size = {640, 480};

        SDL_Log("Create window with size %dx%d.", size.width, size.height);
        auto window = SDL_CreateWindow(data_->name.c_str(),
                                       pos.x, pos.y,
                                       size.width, size.height,
                                       params.sdl_flags);

        if (params.full_screen_mode && params.sdl_flags)
        {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(params.full_screen_mode.display,
                                   params.full_screen_mode.mode,
                                   &mode) >= 0)
            {
                SDL_Log("Set fullscreen mode: %d:%d.",
                        params.full_screen_mode.display,
                        params.full_screen_mode.mode);
                SDL_SetWindowDisplayMode(window, &mode);
            }
        }

        return window;
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

        data_->event_loop->on_update(*this);

        if (data_->event_loop->should_redraw())
        {
            data_->event_loop->clear_redraw();
            data_->event_loop->on_draw(*this);
            SDL_GL_SwapWindow(window());
        }

        if (data_->event_loop_mode == EventLoopMode::WAIT_FOR_EVENTS
            && !data_->event_loop->should_redraw())
        {
            SDL_WaitEvent(&event);
            process_event(event);
        }
    }

    const EventLoop* SdlApplication::event_loop() const
    {
        return data_->event_loop.get();
    }

    EventLoop* SdlApplication::event_loop()
    {
        return data_->event_loop.get();
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
        {
            SDL_Log("Can't use WAIT_FOR_EVENTS with Emscripten.");
            return;
        }
        #endif
        data_->event_loop_mode = mode;
    }

    EventLoop& SdlApplication::callbacks()
    {
        return *data_->event_loop;
    }

    const EventLoop& SdlApplication::callbacks() const
    {
        return *data_->event_loop;
    }

    float aspect_ratio(const SdlApplication& app)
    {
        auto [w, h] = app.window_size();
        return float(w) / float(h);
    }

    SwapInterval swap_interval(const SdlApplication&)
    {
        return SwapInterval(SDL_GL_GetSwapInterval());
    }

    void set_swap_interval(const SdlApplication&, SwapInterval interval)
    {
        auto result = SDL_GL_SetSwapInterval(int(interval));
        if (result == -1 && interval == SwapInterval::ADAPTIVE_VSYNC_OR_VSYNC)
            result = SDL_GL_SetSwapInterval(1);
        if (result == -1)
            THROW_SDL_ERROR();
    }
}
