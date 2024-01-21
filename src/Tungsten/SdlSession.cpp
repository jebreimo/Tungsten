//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlSession.hpp"
#include "Tungsten/TungstenException.hpp"
#include <stdexcept>

namespace Tungsten
{
    SdlSession::SdlSession(uint32_t flags)
            : active_(SDL_Init(flags) == 0)
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

    void configure_sdl(const SdlSession& session,
                       const SdlConfiguration& configuration)
    {
        if (!session.is_active())
            TUNGSTEN_THROW("SDL is not initialized.");
        if (configuration.enable_touch_events)
            SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
    }

    uint32_t get_sdl_init_flags(const SdlConfiguration& configuration)
    {
        uint32_t flags = SDL_INIT_VIDEO;
        if (configuration.enable_timers)
            flags |= SDL_INIT_TIMER;
        return flags;
    }
}
