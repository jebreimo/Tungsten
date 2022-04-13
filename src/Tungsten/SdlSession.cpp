//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/SdlSession.hpp"
#include <stdexcept>

namespace Tungsten
{
    SdlSession::SdlSession(int flags)
            : active_(SDL_Init(flags) == 0)
    {
        if (!active_)
            throw std::runtime_error(SDL_GetError());
    }

    SdlSession::SdlSession(SdlSession&& other)
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
}
