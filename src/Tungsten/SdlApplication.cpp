//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlApplication.hpp"

#include <iostream>
#include <Argos/ArgumentParser.hpp>
#include <Tungsten/GlVersion.hpp>

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

        const char* getPixelFormatName(int format)
        {
            switch (format)
            {
            case SDL_PIXELFORMAT_ABGR1555:
                return "ABGR1555";
            case SDL_PIXELFORMAT_ABGR4444:
                return "ABGR4444";
            case SDL_PIXELFORMAT_ABGR8888:
                return "ABGR8888";
            case SDL_PIXELFORMAT_ARGB1555:
                return "ARGB1555";
            case SDL_PIXELFORMAT_ARGB2101010:
                return "ARGB2101010";
            case SDL_PIXELFORMAT_ARGB4444:
                return "ARGB4444";
            case SDL_PIXELFORMAT_ARGB8888:
                return "ARGB8888";
            case SDL_PIXELFORMAT_BGR24:
                return "BGR24";
            case SDL_PIXELFORMAT_BGR555:
                return "BGR555";
            case SDL_PIXELFORMAT_BGR565:
                return "BGR565";
            case SDL_PIXELFORMAT_BGR888:
                return "BGR888";
            case SDL_PIXELFORMAT_BGRA4444:
                return "BGRA4444";
            case SDL_PIXELFORMAT_BGRA5551:
                return "BGRA5551";
            case SDL_PIXELFORMAT_BGRA8888:
                return "BGRA8888";
            case SDL_PIXELFORMAT_BGRX8888:
                return "BGRX8888";
            case SDL_PIXELFORMAT_INDEX1LSB:
                return "INDEX1LSB";
            case SDL_PIXELFORMAT_INDEX1MSB:
                return "INDEX1MSB";
            case SDL_PIXELFORMAT_INDEX4LSB:
                return "INDEX4LSB";
            case SDL_PIXELFORMAT_INDEX4MSB:
                return "INDEX4MSB";
            case SDL_PIXELFORMAT_INDEX8:
                return "INDEX8";
            case SDL_PIXELFORMAT_IYUV:
                return "IYUV";
            case SDL_PIXELFORMAT_NV12:
                return "NV12";
            case SDL_PIXELFORMAT_NV21:
                return "NV21";
            case SDL_PIXELFORMAT_RGB24:
                return "RGB24";
            case SDL_PIXELFORMAT_RGB332:
                return "RGB332";
            case SDL_PIXELFORMAT_RGB444:
                return "RGB444";
            case SDL_PIXELFORMAT_RGB555:
                return "RGB555";
            case SDL_PIXELFORMAT_RGB565:
                return "RGB565";
            case SDL_PIXELFORMAT_RGB888:
                return "RGB888";
            case SDL_PIXELFORMAT_RGBA4444:
                return "RGBA4444";
            case SDL_PIXELFORMAT_RGBA5551:
                return "RGBA5551";
            case SDL_PIXELFORMAT_RGBA8888:
                return "RGBA8888";
            case SDL_PIXELFORMAT_RGBX8888:
                return "RGBX8888";
            case SDL_PIXELFORMAT_UYVY:
                return "UYVY";
            case SDL_PIXELFORMAT_YUY2:
                return "YUY2";
            case SDL_PIXELFORMAT_YV12:
                return "YV12";
            case SDL_PIXELFORMAT_YVYU:
                return "YVYU";
            default:
                return "UNKNOWN";
            }
        }

        void printDisplayMode(std::ostream& stream,
                              int displayIndex,
                              int modeIndex,
                              const SDL_DisplayMode& mode)
        {
            stream << "Display " << displayIndex
                   << ", mode " << modeIndex
                   << ": " << getPixelFormatName(mode.format)
                   << " " << mode.w << "x" << mode.h
                   << " " << mode.refresh_rate << "Hz\n";
        }

        void printDisplayModes(std::ostream& stream)
        {
            int numDisplays = SDL_GetNumVideoDisplays();
            for (int d = 0; d < numDisplays; ++d)
            {
                if (auto name = SDL_GetDisplayName(d))
                    stream << "Display " << d << ": " << name << "\n";
                int numModes = SDL_GetNumDisplayModes(d);
                for (int m = 0; m < numModes; ++m)
                {
                    SDL_DisplayMode mode = {};
                    if (SDL_GetDisplayMode(d, m, &mode) == 0)
                        printDisplayMode(stream, d, m, mode);
                }
            }
        }
    }

    SdlApplication::SdlApplication(
        std::string name,
        std::unique_ptr<EventLoopCallbacks> callbacks)
        : m_Name(move(name)),
          m_Callbacks(move(callbacks)),
          m_WindowParameters(getDefaultWindowParameters())
    {}

    SdlApplication::~SdlApplication() = default;

    void SdlApplication::parseCommandLineOptions(int& argc, char**& argv)
    {
        using namespace Argos;
        auto args = makeArgParser().parse(argc, argv);
        if (args.value("--list-screen-modes").asBool())
        {
            if (!m_Session)
                m_Session = SdlSession(SDL_INIT_VIDEO);
            printDisplayModes(std::cout);
            auto ver = getSdlGlVersion();
            std::cout << "Version: " << ver.profile << " "
                      << ver.majorVersion << "." << ver.minorVersion << "\n";
            exit(0);
        }

        auto size = args.value("--screensize").split('x', 2, 2)
            .asInts({640, 480});
        m_WindowParameters.windowSize = {size[0], size[1]};
        if (args.value("--fullscreen").asBool())
        {
            auto mode = getClosestDisplayMode(m_WindowParameters.windowSize);
            m_WindowParameters.fullScreenMode = mode.first;
            m_WindowParameters.sdlFlags |= SDL_WINDOW_FULLSCREEN;
        }
    }

    void SdlApplication::run()
    {
        initialize(m_WindowParameters);
        m_Callbacks->onStartup(*this);
        m_IsRunning = true;
        eventLoop();
        m_IsRunning = false;
        m_Callbacks->onShutdown(*this);
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

    void SdlApplication::setWindow(SDL_Window* value)
    {
        m_Window = value;
    }

    std::pair<int, int> SdlApplication::windowSize() const
    {
        int w, h;
        SDL_GetWindowSize(window(), &w, &h);
        return {w, h};
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

    void SdlApplication::initialize(const WindowParameters& windowParams)
    {
        if (!m_Session)
            m_Session = SdlSession(SDL_INIT_VIDEO);
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
        auto& pos = winParams.windowPos;
        auto& size = winParams.windowSize;
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

    std::pair<FullScreenMode, WindowSize>
    SdlApplication::getClosestDisplayMode(WindowSize size, int displayIndex)
    {
        if (!m_Session)
            m_Session = SdlSession(SDL_INIT_VIDEO);
        int bestMode = -1;
        WindowSize bestSize(-1, -1);
        int numModes = SDL_GetNumDisplayModes(displayIndex);
        for (int m = 0; m < numModes; ++m)
        {
            SDL_DisplayMode mode = {};
            if (SDL_GetDisplayMode(displayIndex, m, &mode) == 0
                && (bestMode == -1
                    || (mode.w >= size.width && mode.h >= size.height
                        && mode.w * mode.h < bestSize.width * bestSize.height)))
            {
                bestMode = m;
                bestSize = {mode.w, mode.h};
            }
        }

        return {{displayIndex, bestMode}, bestSize};
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
            if (!m_Callbacks->onEvent(*this, event))
                processEvent(event);
        }
        m_Callbacks->onUpdate(*this);
        m_Callbacks->onDraw(*this);
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

    EventLoopCallbacks& SdlApplication::callbacks()
    {
        return *m_Callbacks;
    }

    const EventLoopCallbacks& SdlApplication::callbacks() const
    {
        return *m_Callbacks;
    }
}
