//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SdlSession.hpp"
#include <memory>
#include <stdexcept>

namespace Tungsten
{
    SdlSession::SdlSession(int flags)
            : m_Active(true)
    {
        m_Active = SDL_Init(flags) == 0;
    }

    SdlSession::SdlSession(SdlSession&& other)
            : m_Active(other.m_Active)
    {
        other.m_Active = false;
    }

    SdlSession::~SdlSession()
    {
        if (m_Active)
            SDL_Quit();
    }

    SdlSession& SdlSession::operator=(SdlSession&& other)
    {
        if (m_Active)
            SDL_Quit();
        m_Active = other.m_Active;
        other.m_Active = false;
        return *this;
    }

    SdlSession SdlSession::create(int flags)
    {
        auto session = SdlSession{flags};
        if (!session.m_Active)
            throw std::runtime_error(SDL_GetError());
        return session;
    }
}
