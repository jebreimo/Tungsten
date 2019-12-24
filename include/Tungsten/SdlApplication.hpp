//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "GlContext.hpp"
#include "WindowParameters.hpp"

#ifdef __EMSCRIPTEN__
    constexpr int TUNGSTEN_GL_PROFILE = SDL_GL_CONTEXT_PROFILE_ES;
    constexpr int TUNGSTEN_GL_MAJOR_VERSION = 2;
    constexpr int TUNGSTEN_GL_MINOR_VERSION = 0;
#else
    constexpr int TUNGSTEN_GL_PROFILE = SDL_GL_CONTEXT_PROFILE_CORE;
    constexpr int TUNGSTEN_GL_MAJOR_VERSION = 3;
    constexpr int TUNGSTEN_GL_MINOR_VERSION = 1;
#endif

namespace Tungsten
{
    class SdlApplication
    {
    public:
        SdlApplication();

        SdlApplication(SdlApplication&&) = default;

        virtual ~SdlApplication();

        SdlApplication& operator=(SdlApplication&&) = default;

        virtual void setup();

        virtual void update();

        virtual void draw();

        virtual void shutdown();

        virtual void run();

        void run(const WindowParameters& params);

        bool isRunning() const;

        void quit();

        SDL_GLContext glContext() const;

        SdlApplication& setGlContext(GlContext&& context);

        int status();

        SdlApplication& setSwapInterval(int interval);

        SDL_Window* window() const;

        void setWindow(SDL_Window* window);

        float aspectRatio() const;
    protected:
        SdlApplication& setGlVersion(int majorVersion, int minorVersion);

        void setStatus(int status);

        virtual bool processEvent(const SDL_Event& event);

        virtual void doInitialize(const WindowParameters& windowParams);

        virtual void doRun();

        virtual void postDraw();

        static SDL_Window* createWindow(const WindowParameters& windowParams);
    private:
        void eventLoop();

        void eventLoopStep();

        #ifdef __EMSCRIPTEN__
        static void emscriptenEventLoopStep(void* arg);
        #endif

        SDL_Window* m_Window = nullptr;
        GlContext m_GlContext = {};
        int m_MajorGlVersion = TUNGSTEN_GL_MAJOR_VERSION;
        int m_MinorGlVersion = TUNGSTEN_GL_MINOR_VERSION;
        int m_Status = 0;
        bool m_IsRunning = false;
    };
}
