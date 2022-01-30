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
            : m_active(SDL_Init(flags) == 0)
    {
        if (!m_active)
            throw std::runtime_error(SDL_GetError());
    }

    SdlSession::SdlSession(SdlSession&& other)
            : m_active(other.m_active)
    {
        other.m_active = false;
    }

    SdlSession::~SdlSession()
    {
        if (m_active)
            SDL_Quit();
    }

    SdlSession& SdlSession::operator=(SdlSession&& other) noexcept
    {
        if (m_active)
            SDL_Quit();
        m_active = other.m_active;
        other.m_active = false;
        return *this;
    }
}
