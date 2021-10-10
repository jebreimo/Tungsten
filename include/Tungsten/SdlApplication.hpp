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
#include <vector>
#include <GL/glew.h>
#include "EventLoop.hpp"
#include "GlContext.hpp"
#include "WindowParameters.hpp"

namespace argos
{
    class ArgumentParser;
    class ParsedArguments;
}

namespace Tungsten
{
    class SdlApplication;

    class SdlApplication
    {
    public:
        SdlApplication(std::string name,
                       std::unique_ptr<EventLoop> eventloop);

        SdlApplication(SdlApplication&&) = default;

        virtual ~SdlApplication();

        SdlApplication& operator=(SdlApplication&&) = default;

        [[nodiscard]]
        const std::string& name() const;

        static void addCommandLineOptions(argos::ArgumentParser& parser);

        void readCommandLineOptions(const argos::ParsedArguments& args);

        void parseCommandLineOptions(int& argc, char**& argv);

        void run();

        [[nodiscard]]
        bool isRunning() const;

        void quit();

        [[nodiscard]]
        SDL_GLContext glContext() const;

        [[nodiscard]]
        int status() const;

        SdlApplication& setSwapInterval(int interval);

        [[nodiscard]]
        SDL_Window* window() const;

        void setWindow(SDL_Window* window);

        [[nodiscard]]
        std::pair<int, int> windowSize() const;

        [[nodiscard]]
        float aspectRatio() const;

        [[nodiscard]]
        const WindowParameters& windowParameters() const;

        void setWindowParameters(const WindowParameters& windowParameters);

        [[nodiscard]]
        const EventLoop& callbacks() const;

        EventLoop& callbacks();
    protected:
        void setStatus(int status);

        bool processEvent(const SDL_Event& event);

        void initialize(const WindowParameters& windowParams);

        SDL_Window* createWindow(const WindowParameters& winParams);
    private:
        void eventLoop();

        void eventLoopStep();

        #ifdef __EMSCRIPTEN__
        static void emscriptenEventLoopStep(void* arg);
        #endif

        std::string m_Name;
        std::unique_ptr<EventLoop> m_EventLoop;
        WindowParameters m_WindowParameters;
        SDL_Window* m_Window = nullptr;
        GlContext m_GlContext = {};
        int m_Status = 0;
        bool m_IsRunning = false;
    };
}
