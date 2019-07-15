//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

namespace Tungsten
{
    class SdlSession
    {
    public:
        SdlSession(SdlSession&& other);

        ~SdlSession();

        SdlSession& operator=(SdlSession&& other);

        static SdlSession create(int flags);
    private:
        SdlSession(int flags);

        int m_Active;
    };
}
