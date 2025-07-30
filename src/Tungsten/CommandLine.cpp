//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "CommandLine.hpp"

#include <iostream>
#include <SDL3/SDL.h>
#include <Argos/ArgumentParser.hpp>
#include <Tungsten/SdlSession.hpp>
#include <Tungsten/SdlApplication.hpp>

#include "Tungsten/SdlDisplay.hpp"

namespace Tungsten
{
    namespace
    {
        const char* get_pixel_format_name(uint32_t format)
        {
            switch (format)
            {
            case SDL_PIXELFORMAT_ABGR1555:
                return "ABGR_1555";
            case SDL_PIXELFORMAT_ABGR4444:
                return "ABGR_4444";
            case SDL_PIXELFORMAT_ABGR8888:
                return "ABGR_8888";
            case SDL_PIXELFORMAT_ARGB1555:
                return "ARGB_1555";
            case SDL_PIXELFORMAT_ARGB2101010:
                return "ARGB_2101010";
            case SDL_PIXELFORMAT_ARGB4444:
                return "ARGB_4444";
            case SDL_PIXELFORMAT_ARGB8888:
                return "ARGB_8888";
            case SDL_PIXELFORMAT_BGR24:
                return "BGR_24";
            case SDL_PIXELFORMAT_XBGR1555:
                return "XBGR_1555";
            case SDL_PIXELFORMAT_BGR565:
                return "BGR_565";
            case SDL_PIXELFORMAT_XBGR8888:
                return "XBGR_8888";
            case SDL_PIXELFORMAT_BGRA4444:
                return "BGRA_4444";
            case SDL_PIXELFORMAT_BGRA5551:
                return "BGRA_5551";
            case SDL_PIXELFORMAT_BGRA8888:
                return "BGRA_8888";
            case SDL_PIXELFORMAT_BGRX8888:
                return "BGRX_8888";
            case SDL_PIXELFORMAT_INDEX1LSB:
                return "INDEX_1_LSB";
            case SDL_PIXELFORMAT_INDEX1MSB:
                return "INDEX_1_MSB";
            case SDL_PIXELFORMAT_INDEX4LSB:
                return "INDEX_4_LSB";
            case SDL_PIXELFORMAT_INDEX4MSB:
                return "INDEX_4_MSB";
            case SDL_PIXELFORMAT_INDEX8:
                return "INDEX_8";
            case SDL_PIXELFORMAT_IYUV:
                return "IYUV";
            case SDL_PIXELFORMAT_NV12:
                return "NV12";
            case SDL_PIXELFORMAT_NV21:
                return "NV21";
            case SDL_PIXELFORMAT_RGB24:
                return "RGB_24";
            case SDL_PIXELFORMAT_RGB332:
                return "RGB_332";
            case SDL_PIXELFORMAT_XRGB4444:
                return "XRGB_4444";
            case SDL_PIXELFORMAT_XRGB1555:
                return "XRGB_1555";
            case SDL_PIXELFORMAT_RGB565:
                return "RGB_565";
            case SDL_PIXELFORMAT_XRGB8888:
                return "XRGB_8888";
            case SDL_PIXELFORMAT_RGBA4444:
                return "RGBA_4444";
            case SDL_PIXELFORMAT_RGBA5551:
                return "RGBA_5551";
            case SDL_PIXELFORMAT_RGBA8888:
                return "RGBA_8888";
            case SDL_PIXELFORMAT_RGBX8888:
                return "RGBX_8888";
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

        void print_display_mode(std::ostream& stream,
                                SDL_DisplayID display_id,
                                int mode_index,
                                const SDL_DisplayMode& mode)
        {
            stream << "display " << display_id
                   << " mode " << mode_index
                   << "  " << get_pixel_format_name(mode.format)
                   << " " << mode.w << "x" << mode.h
                   << " " << mode.refresh_rate << "Hz\n";
        }

        void print_display_modes(std::ostream& stream)
        {
            int num_displays = 0;
            const auto display_ids = get_sdl_display_ids();
            for (auto id = display_ids.get(); *id; ++id)
            {
                if (const auto name = SDL_GetDisplayName(*id))
                    stream << "Display " << *id << ": " << name << "\n";

                auto modes = get_sdl_display_modes(*id);
                int n = 0;
                for (const auto* mode_ptr = modes.get(); *mode_ptr; ++mode_ptr)
                    print_display_mode(stream, *id, n++, **mode_ptr);
            }
        }
    }

    void add_command_line_options(argos::ArgumentParser& parser)
    {
        using namespace argos;
        parser.allow_abbreviated_options(true)
            .current_section("DISPLAY OPTIONS")
            .add(Option{"--windowpos"}.argument("<X>,<Y>")
                     .help("Set the window position (top left corner)."))
            .add(Option{"--windowsize"}.argument("<HOR>x<VER>")
                     .help("Set the window size."))
            .add(Option{"--fullscreen"}
                     .constant("F")
                     .help("Start program in the default fullscreen mode."))
            .add(Option{"--fullscreen="}.argument("<MODE>")
                     .alias("--fullscreen")
                     .help("Start program in the given fullscreen mode."
                           " MODE is a pair of integers separated by a colon,"
                           " e.g. '0:5'. Use --listmodes to list available"
                           " modes."))
            .add(Option{"--window"}
                     .alias("--fullscreen").constant("W")
                     .help("Start program in window mode. (Default)"))
            .add(Option{"--listmodes"}.type(OptionType::STOP)
                     .help("Display a list of the available fullscreen"
                           " modes and quit."));
    }

    void read_command_line_options(const argos::ParsedArguments& args,
                                   SdlApplication& app)
    {
        if (args.value("--listmodes").as_bool())
        {
            SdlSession session(SDL_INIT_VIDEO);
            print_display_modes(std::cout);
            exit(0);
        }

        WindowParameters wp;

        if (auto mode_arg = args.value("--fullscreen"))
        {
            if (mode_arg.as_string() == "F")
            {
                wp.sdl_flags |= SDL_WINDOW_FULLSCREEN;
            }
            else if (mode_arg.as_string() != "W")
            {
                auto mode = mode_arg.split(':', 2, 2).as_ints();
                wp.full_screen_mode = {mode[0], mode[1]};
                wp.sdl_flags |= SDL_WINDOW_FULLSCREEN;
            }
        }

        if (auto window_size_arg = args.value("--windowsize"))
        {
            auto size = window_size_arg.split('x', 2, 2).as_ints();
            wp.window_size = {size[0], size[1]};
        }

        if (auto window_pos_arg = args.value("--windowpos"))
        {
            auto pos = window_pos_arg.split(',', 2, 2).as_ints();
            wp.window_size = {pos[0], pos[1]};
        }

        app.set_window_parameters(wp);
    }

    void parse_command_line_options(int& argc, char**& argv,
                                    SdlApplication& app)
    {
        using namespace argos;
        ArgumentParser parser(app.name());
        add_command_line_options(parser);
        auto args = parser.parse(argc, argv);
        read_command_line_options(args, app);
        args.filter_parsed_arguments(argc, argv);
    }
}
