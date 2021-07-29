//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "CommandLine.hpp"

#include <SDL2/SDL.h>
#include <Argos/ArgumentParser.hpp>
#include <Tungsten/SdlSession.hpp>
#include <iostream>
#include <Tungsten/GlVersion.hpp>
#include <Tungsten/SdlApplication.hpp>

namespace Tungsten
{
    namespace
    {
        Argos::ArgumentParser makeArgParser(const std::string_view& appName,
                                            bool partialParse)
        {
            using namespace Argos;
            ArgumentParser p(appName);
            if (partialParse)
            {
                p.text(TextId::USAGE, {})
                    .ignoreUndefinedArguments(true)
                    .ignoreUndefinedArguments(true)
                    .generateHelpOption(false);
            }
            return p.allowAbbreviatedOptions(true)
                .add(Option{"--display"}.argument("NUM")
                         .text("Set display for fullscreen mode."
                               " (Default: 0)"))
                .add(Option{"--mode"}.argument("NUM")
                         .text("Set mode for fullscreen mode. The normal"
                               " desktop mode is used if none is given."))
                .add(Option{"--windowsize"}.argument("<HOR>x<VER>")
                         .text("Set the window size."
                               " Ignored if --mode is given."))
                .add(Option{"--fullscreen"}
                         .text("Start program in fullscreen mode."))
                .add(Option{"--window"}.value("--fullscreen").constant(false)
                         .text("Start program in window mode. (Default)"))
                .add(Option{"--list-modes"}.type(OptionType::STOP)
                         .text("Display a list of the available fullscreen"
                               " modes and quit."))
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

        std::pair<int, int> getScreenResolution(int display, int mode)
        {
            SDL_DisplayMode modeInfo = {};
            if (SDL_GetDisplayMode(display, mode, &modeInfo) != 0)
                return {};
            return {modeInfo.w, modeInfo.h};
        }
    }

    void parseCommandLineOptions(int& argc, char**& argv,
                                 SdlApplication& app,
                                 bool partialParse)
    {
        using namespace Argos;
        auto args = makeArgParser(app.name(), partialParse).parse(argc, argv);
        if (args.value("--list-modes").asBool())
        {
            SdlSession session(SDL_INIT_VIDEO);
            printDisplayModes(std::cout);
            exit(0);
        }

        WindowParameters wp;
        auto display = args.value("--display");
        auto mode = args.value("--mode");
        if (display && !mode)
        {
            args.value("--display")
                .error("'--mode' must also be given for '--display' to"
                       " have any effect.");
        }
        wp.fullScreenMode = {display.asInt(-1), mode.asInt(-1)};
        if (args.value("--fullscreen").asBool())
        {
            if (wp.fullScreenMode)
            {
                wp.sdlFlags |= SDL_WINDOW_FULLSCREEN;
                auto [w, h] = getScreenResolution(
                    wp.fullScreenMode.displayIndex,
                    wp.fullScreenMode.modeIndex);
                wp.windowSize = {w, h};
            }
            else
            {
                wp.sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
        }

        if (auto windowSize = args.value("--windowsize"))
        {
            auto size = windowSize.split('x', 2, 2).asInts({640, 480});
            wp.windowSize = {size[0], size[1]};
            app.setWindowParameters(wp);
        }

        if (partialParse)
            args.filterParsedArguments(argc, argv);
    }

    void printCommandLineHelp(std::ostream& stream)
    {
        makeArgParser({}, true).stream(&stream).writeHelpText();
    }
}
