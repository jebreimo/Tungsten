//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-07-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <SDL3/SDL_video.h>

namespace Tungsten
{
    template <typename T>
    struct SdlPtrDeleter
    {
        void operator()(T* ptr) const
        {
            if (ptr)
                SDL_free(ptr);
        }
    };

    template <typename T>
    using SdlPtr = std::unique_ptr<T, SdlPtrDeleter<T>>;

    using SdlDisplayIdPtr = SdlPtr<SDL_DisplayID>;
    SdlDisplayIdPtr get_sdl_display_ids();

    using SdlDisplayModesPtr = SdlPtr<SDL_DisplayMode*>;

    SdlDisplayModesPtr get_sdl_display_modes(SDL_DisplayID id);

    const SDL_DisplayMode* get_sdl_display_mode(SDL_DisplayMode** modes, int index);
}
