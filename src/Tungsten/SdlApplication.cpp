//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlApplication.hpp"
#include "SdlSession.hpp"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

namespace Tungsten
{
    SdlApplication::SdlApplication() = default;

    SdlApplication::~SdlApplication() = default;

    void SdlApplication::setup()
    {}

    void SdlApplication::update()
    {}

    void SdlApplication::draw()
    {}

    void SdlApplication::shutdown()
    {}

    void SdlApplication::run(const WindowParameters& windowParams)
    {
        auto session = SdlSession::create(SDL_INIT_VIDEO);
        doInitialize(windowParams);
        doRun();
    }

    void SdlApplication::run()
    {
        run(WindowParameters());
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

    SdlApplication& SdlApplication::setGlContext(GlContext&& value)
    {
        m_GlContext = std::move(value);
        return *this;
    }

    int SdlApplication::status()
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

    SdlApplication& SdlApplication::setGlVersion(int majorVersion,
                                                 int minorVersion)
    {
        m_MajorGlVersion = majorVersion;
        m_MinorGlVersion = minorVersion;
        return *this;
    }

    void SdlApplication::setWindow(SDL_Window* value)
    {
        m_Window = value;
    }

    float SdlApplication::aspectRatio() const
    {
        int w, h;
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

    void SdlApplication::doInitialize(const WindowParameters& windowParams)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            TUNGSTEN_GL_PROFILE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, m_MajorGlVersion);
        if (m_MinorGlVersion)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, m_MinorGlVersion);

        auto tmpWindowParams = windowParams;
        tmpWindowParams.flags |= SDL_WINDOW_OPENGL;

        setWindow(createWindow(tmpWindowParams));

        m_GlContext = GlContext::create(window());

        glewExperimental = GL_TRUE;
        glewInit();
    }

    void SdlApplication::doRun()
    {
        setup();
        m_IsRunning = true;
        eventLoop();
        m_IsRunning = false;
        shutdown();
    }

    void SdlApplication::postDraw()
    {
        SDL_GL_SwapWindow(window());
    }

    SDL_Window* SdlApplication::createWindow(const WindowParameters& windowParams)
    {
        auto& wr = windowParams.windowRectangle;
        return SDL_CreateWindow(windowParams.title.c_str(), wr.x, wr.y,
                                wr.width, wr.height, windowParams.flags);
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
            processEvent(event);
        update();
        draw();
        postDraw();
    }
}
