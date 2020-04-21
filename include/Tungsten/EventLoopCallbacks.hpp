//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL2/SDL_events.h>

namespace Tungsten
{
    class SdlApplication;

    class EventLoopCallbacks
    {
    public:
        virtual ~EventLoopCallbacks() = default;

        virtual void onStartup(SdlApplication& app)
        {}

        virtual bool onEvent(SdlApplication& app, const SDL_Event& event)
        {
            return false;
        }

        virtual void onUpdate(SdlApplication& app)
        {}

        virtual void onDraw(SdlApplication& app)
        {}

        virtual void onShutdown(SdlApplication& app)
        {}
    };
}
