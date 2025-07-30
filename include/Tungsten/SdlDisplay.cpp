//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-07-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SdlDisplay.hpp"

#include "TungstenException.hpp"

namespace Tungsten
{
    SdlDisplayIdPtr get_sdl_display_ids()
    {
        auto* display_ids = SDL_GetDisplays(nullptr);
        if (!display_ids)
            THROW_SDL_ERROR();

        return {display_ids, SdlPtrDeleter<SDL_DisplayID>()};
    }

    SdlDisplayModesPtr get_sdl_display_modes(SDL_DisplayID id)
    {
        auto* modes = SDL_GetFullscreenDisplayModes(id, nullptr);
        if (!modes)
            THROW_SDL_ERROR();

        return {modes, SdlPtrDeleter<SDL_DisplayMode*>()};
    }

    const SDL_DisplayMode* get_sdl_display_mode(SDL_DisplayMode** modes, int index)
    {
        for (const auto* mode_ptr = modes; *mode_ptr; ++mode_ptr)
        {
            if (index-- == 0)
                return *mode_ptr;
        }

        TUNGSTEN_THROW("No such display mode: " + std::to_string(index));
    }
}
