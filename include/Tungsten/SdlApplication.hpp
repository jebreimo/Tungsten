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
    protected:
        SdlApplication& setGlVersion(int majorVersion, int minorVersion);

        void setStatus(int status);

        void eventLoop();

        virtual bool processEvent(const SDL_Event& event);

        virtual void doInitialize(const WindowParameters& windowParams);

        virtual void doRun();

        virtual void postDraw();

        static SDL_Window* createWindow(const WindowParameters& windowParams);

    private:
        SDL_Window* m_Window;
        GlContext m_GlContext;
        int m_MajorGlVersion = 3;
        int m_MinorGlVersion = 1;
        int m_Status;
        bool m_IsRunning;
    };
}
