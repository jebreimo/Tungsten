//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlApplication.hpp"

#include <Argos/ArgumentParser.hpp>
#include "Tungsten/EventLoop.hpp"
#include "Tungsten/GlVersion.hpp"
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

        WindowParameters getDefaultWindowParameters()
        {
            int width = canvas_get_width();
            int height = canvas_get_height();

            WindowParameters params;
            if (width > 0 && height > 0)
                params.window_size = {width, height};
            return params;
        }

        #else

        WindowParameters getDefaultWindowParameters()
        {
            return {};
        }

        #endif
    }

    SdlApplication::SdlApplication(
        std::string name,
        std::unique_ptr<EventLoop> event_loop)
        : m_name(move(name)),
          m_event_loop(move(event_loop)),
          m_window_parameters(getDefaultWindowParameters())
    {}

    SdlApplication::~SdlApplication() = default;

    const std::string& SdlApplication::name() const
    {
        return m_name;
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

    void SdlApplication::run()
    {
        if (!m_event_loop)
            TUNGSTEN_THROW("No eventloop.");
        SdlSession session(SDL_INIT_VIDEO);
        initialize(m_window_parameters);
        m_event_loop->on_startup(*this);
        m_is_running = true;
        event_loop();
        m_is_running = false;
        m_event_loop->on_shutdown(*this);
    }

    bool SdlApplication::is_running() const
    {
        return m_is_running;
    }

    void SdlApplication::quit()
    {
        set_status(1);
    }

    SDL_GLContext SdlApplication::gl_context() const
    {
        return m_gl_context;
    }

    int SdlApplication::status() const
    {
        return m_status;
    }

    void SdlApplication::set_status(int status)
    {
        m_status = status;
    }

    SdlApplication& SdlApplication::set_swap_interval(int interval)
    {
        SDL_GL_SetSwapInterval(interval);
        return *this;
    }

    SDL_Window* SdlApplication::window() const
    {
        return m_window;
    }

    void SdlApplication::setWindow(SDL_Window* value)
    {
        m_window = value;
    }

    std::pair<int, int> SdlApplication::window_size() const
    {
        int w = 0, h = 0;
        SDL_GetWindowSize(window(), &w, &h);
        return {w, h};
    }

    float SdlApplication::aspect_ratio() const
    {
        int w = 0, h = 0;
        SDL_GetWindowSize(window(), &w, &h);
        return float(w) / float(h);
    }

    bool SdlApplication::process_event(const SDL_Event& event)
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
                glViewport(0, 0, event.window.data1, event.window.data2);
            break;
        default:
            break;
        }
        return true;
    }

    void SdlApplication::initialize(const WindowParameters& window_parameters)
    {
        set_sdl_gl_version(get_default_gl_version(GlVersionCode::ES_2));
        auto tmpWindowParams = window_parameters;
        tmpWindowParams.sdl_flags |= SDL_WINDOW_OPENGL;

        setWindow(create_window(tmpWindowParams));

        m_gl_context = GlContext::create(window());

        glewExperimental = GL_TRUE;
        glewInit();
    }

    SDL_Window*
    SdlApplication::create_window(const WindowParameters& window_parameters)
    {
        const auto& pos = window_parameters.window_pos;
        auto size = window_parameters.window_size;
        if (window_parameters.full_screen_mode && !size)
        {
            SDL_DisplayMode modeInfo = {};
            if (SDL_GetDisplayMode(window_parameters.full_screen_mode.display,
                                   window_parameters.full_screen_mode.mode,
                                   &modeInfo) >= 0)
            {
                size = {modeInfo.w, modeInfo.h};
            }
        }

        if (!size)
            size = window_parameters.default_window_size;

        SDL_Log("Create window with size %dx%d.", size.width, size.height);
        auto window = SDL_CreateWindow(m_name.c_str(),
                                       pos.x, pos.y,
                                       size.width, size.height,
                                       window_parameters.sdl_flags);

        if (window_parameters.full_screen_mode && window_parameters.sdl_flags)
        {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(window_parameters.full_screen_mode.display,
                                   window_parameters.full_screen_mode.mode,
                                   &mode) >= 0)
            {
                SDL_Log("Set fullscreen mode: %d:%d.",
                        window_parameters.full_screen_mode.display,
                        window_parameters.full_screen_mode.mode);
                SDL_SetWindowDisplayMode(window, &mode);
            }
        }

        return window;
    }

    #ifdef __EMSCRIPTEN__

    void SdlApplication::eventLoop()
    {
        const int simulate_infinite_loop = 1; // call the function repeatedly
        const int fps = -1; // call the function as fast as the browser wants to render (typically 60fps)
        emscripten_set_main_loop_arg(&SdlApplication::emscriptenEventLoopStep,
                                     this,
                                     fps,
                                     simulate_infinite_loop);
    }

    void SdlApplication::emscriptenEventLoopStep(void* arg)
    {
        auto app = static_cast<SdlApplication*>(arg);
        app->eventLoopStep();
    }

    #else

    void SdlApplication::event_loop()
    {
        while (!status())
            event_loop_step();
    }

    #endif

    void SdlApplication::event_loop_step()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (!m_event_loop->on_event(*this, event))
                process_event(event);
        }
        m_event_loop->on_update(*this);
        m_event_loop->on_draw(*this);
        SDL_GL_SwapWindow(window());
    }

    const WindowParameters& SdlApplication::window_parameters() const
    {
        return m_window_parameters;
    }

    void SdlApplication::set_window_parameters(const WindowParameters& window_parameters)
    {
        m_window_parameters = window_parameters;
    }

    EventLoop& SdlApplication::callbacks()
    {
        return *m_event_loop;
    }

    const EventLoop& SdlApplication::callbacks() const
    {
        return *m_event_loop;
    }
}
