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
                params.windowRectangle = WindowRectangle(width, height);
            return params;
        }

        #else

        WindowParameters getDefaultWindowParameters()
        {
            return {};
        }

        #endif

        Argos::ArgumentParser makeArgParser()
        {
            using namespace Argos;
            return ArgumentParser()
                .text(TextId::USAGE_TITLE, {})
                .text(TextId::USAGE, {})
                .text(TextId::ERROR_USAGE, {})
                .ignoreUndefinedArguments(true)
                .ignoreUndefinedArguments(true)
                .allowAbbreviatedOptions(true)
                .add(Option{"--fullscreen"})
                .add(Option{"--screensize"}.argument("<HOR>x<VER>"))
                .add(Option{"--list-screen-modes"}.type(OptionType::STOP))
                .move();
        }
    }

    SdlApplication::SdlApplication(
        std::string name,
        std::unique_ptr<EventLoop> eventloop)
        : m_Name(move(name)),
          m_EventLoop(move(eventloop)),
          m_WindowParameters(getDefaultWindowParameters())
    {}

    SdlApplication::~SdlApplication() = default;

    const std::string& SdlApplication::name() const
    {
        return m_Name;
    }

    void SdlApplication::parseCommandLineOptions(int& argc, char**& argv,
                                                 bool partialParse)
    {
        Tungsten::parseCommandLineOptions(argc, argv, *this, partialParse);
    }

    void SdlApplication::printCommandLineHelp(std::ostream& stream) const
    {
        Tungsten::printCommandLineHelp(stream);
    }

    void SdlApplication::run()
    {
        if (!m_EventLoop)
            TUNGSTEN_THROW("No eventloop.");
        SdlSession session(SDL_INIT_VIDEO);
        initialize(m_WindowParameters);
        m_EventLoop->onStartup(*this);
        m_IsRunning = true;
        eventLoop();
        m_IsRunning = false;
        m_EventLoop->onShutdown(*this);
    }

    bool SdlApplication::isRunning() const
    {
        return m_IsRunning;
    }

    void SdlApplication::quit()
    {
        setStatus(1);
    }

    SDL_GLContext SdlApplication::glContext() const
    {
        return m_GlContext;
    }

    int SdlApplication::status() const
    {
        return m_Status;
    }

    void SdlApplication::setStatus(int status)
    {
        m_Status = status;
    }

    SdlApplication& SdlApplication::setSwapInterval(int interval)
    {
        SDL_GL_SetSwapInterval(interval);
        return *this;
    }

    SDL_Window* SdlApplication::window() const
    {
        return m_Window;
    }

    void SdlApplication::setWindow(SDL_Window* value)
    {
        m_Window = value;
    }

    std::pair<int, int> SdlApplication::windowSize() const
    {
        int w = 0, h = 0;
        SDL_GetWindowSize(window(), &w, &h);
        return {w, h};
    }

    float SdlApplication::aspectRatio() const
    {
        int w = 0, h = 0;
        SDL_GetWindowSize(window(), &w, &h);
        return float(w) / float(h);
    }

    bool SdlApplication::processEvent(const SDL_Event& event)
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

    void SdlApplication::initialize(const WindowParameters& windowParams)
    {
        setSdlGlVersion(getDefaultGlVersion(GlVersionCode::ES_2));
        auto tmpWindowParams = windowParams;
        tmpWindowParams.sdlFlags |= SDL_WINDOW_OPENGL;

        setWindow(createWindow(tmpWindowParams));

        m_GlContext = GlContext::create(window());

        glewExperimental = GL_TRUE;
        glewInit();
    }

    SDL_Window* SdlApplication::createWindow(const WindowParameters& winParams)
    {
        SDL_Log("createWindow");
        const auto& pos = winParams.windowPos;
        const auto& size = winParams.windowSize;
        auto window = SDL_CreateWindow(winParams.title.c_str(),
                                       pos.x, pos.y,
                                       size.width, size.height,
                                       winParams.sdlFlags);
        if (winParams.fullScreenMode && winParams.sdlFlags)
        {
            SDL_Log("fullscreen");

            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(winParams.fullScreenMode.displayIndex, winParams.fullScreenMode.modeIndex, &mode) >= 0)
            {
                SDL_SetWindowDisplayMode(window, &mode);
                SDL_Log("set fullscreen");
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

    void SdlApplication::eventLoop()
    {
        while (!status())
            eventLoopStep();
    }

    #endif

    void SdlApplication::eventLoopStep()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (!m_EventLoop->onEvent(*this, event))
                processEvent(event);
        }
        m_EventLoop->onUpdate(*this);
        m_EventLoop->onDraw(*this);
        SDL_GL_SwapWindow(window());
    }

    const WindowParameters& SdlApplication::windowParameters() const
    {
        return m_WindowParameters;
    }

    void SdlApplication::setWindowParameters(const WindowParameters& windowParameters)
    {
        m_WindowParameters = windowParameters;
    }

    EventLoop& SdlApplication::callbacks()
    {
        return *m_EventLoop;
    }

    const EventLoop& SdlApplication::callbacks() const
    {
        return *m_EventLoop;
    }
}
