//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlSession.hpp"

#include <stdexcept>
#include <SDL3/SDL.h>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    SdlSession::SdlSession(uint32_t flags)
            : active_(SDL_Init(flags))
    {
        if (!active_)
            throw std::runtime_error(SDL_GetError());
    }

    SdlSession::SdlSession(SdlSession&& other) noexcept
            : active_(other.active_)
    {
        other.active_ = false;
    }

    SdlSession::~SdlSession()
    {
        if (active_)
            SDL_Quit();
    }

    SdlSession& SdlSession::operator=(SdlSession&& other) noexcept
    {
        if (active_)
            SDL_Quit();
        active_ = other.active_;
        other.active_ = false;
        return *this;
    }

    bool SdlSession::is_active() const
    {
        return active_;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    bool SdlSession::touch_events_enabled() const
    {
        return strcmp(SDL_GetHint(SDL_HINT_MOUSE_TOUCH_EVENTS), "1") == 0;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void SdlSession::set_touch_events_enabled(bool enabled)
    {
        if (!SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, enabled ? "1" : "0"))
            THROW_SDL_ERROR();
    }
}
