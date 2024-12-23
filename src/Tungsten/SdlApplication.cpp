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
#include "Tungsten/GlContext.hpp"
#include "Tungsten/GlParameters.hpp"
#include "Tungsten/TungstenException.hpp"
#include "EventThrottlers/MouseWheelEventMerger.hpp"
#include "EventThrottlers/MultiGestureEventMerger.hpp"
#include "EventThrottlers/EventThrottler.hpp"
#include "CommandLine.hpp"

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
        {}

        EventLoop* event_loop = nullptr;
        SdlSession* session = nullptr;

        std::string name;
        WindowParameters window_parameters;
        EventLoopMode event_loop_mode = EventLoopMode::UPDATE_CONTINUOUSLY;
        int status = 0;
        SDL_Window* window = nullptr;
        GlContext gl_context;
        std::unordered_map<SDL_EventType, EventThrottler> event_throttlers;
        bool enable_touch_events = false;
        bool enable_timers = false;
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

    void SdlApplication::throttle_events(SDL_EventType event, uint32_t msecs)
    {
        if (msecs == 0)
        {
            data_->event_throttlers.erase(event);
            return;
        }

        switch (event)
        {
        case SDL_MULTIGESTURE:
            data_->event_throttlers[event] = EventThrottler(
                std::make_unique<MultiGestureEventMerger>(), msecs);
            break;
        case SDL_MOUSEWHEEL:
            data_->event_throttlers[event] = EventThrottler(
                std::make_unique<MouseWheelEventMerger>(), msecs);
            break;
        default:
            TUNGSTEN_THROW("Unsupported event type: " + std::to_string(event));
        }
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

    Size2D SdlApplication::window_size() const
    {
        if (!window())
            TUNGSTEN_THROW("window is NULL!");

        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window(), &w, &h);
        return {w, h};
    }

    void SdlApplication::process_event(const SDL_Event& event)
    {
        if (!data_->event_throttlers.empty())
        {
            auto it = data_->event_throttlers.find(SDL_EventType(event.type));
            if (it != data_->event_throttlers.end())
            {
                auto& throttler = it->second;
                if (!throttler.update(event))
                {
                    process_event_for_real(throttler.event(event.common.timestamp));
                    throttler.clear();
                    throttler.update(event);
                }
                return;
            }
        }
        process_event_for_real(event);
    }

    void SdlApplication::process_event_for_real(const SDL_Event& event)
    {
        if (!data_->event_loop->on_event(event))
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
                    glFinish();
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

        if (!data_->event_throttlers.empty())
        {
            const auto time = SDL_GetTicks();
            for (auto& [_, throttler]: data_->event_throttlers)
            {
                if (throttler.is_due(time))
                {
                    process_event_for_real(throttler.event(SDL_GetTicks()));
                    throttler.clear();
                }
            }
        }

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

    bool SdlApplication::sdl_timers_enabled() const
    {
        return data_->enable_timers;
    }

    void SdlApplication::set_sdl_timers_enabled(bool value)
    {
        if (is_running())
            TUNGSTEN_THROW("Can not change SDL timers while the application is running.");
        data_->enable_timers = value;
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
        uint32_t flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
        if (data_->enable_timers)
            flags |= SDL_INIT_TIMER;
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
