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

    class EventLoop
    {
    public:
        virtual ~EventLoop() = default;

        virtual void on_startup(SdlApplication& app)
        {}

        virtual bool on_event(SdlApplication& app, const SDL_Event& event)
        {
            return false;
        }

        virtual void on_update(SdlApplication& app)
        {}

        virtual void on_draw(SdlApplication& app)
        {}

        virtual void on_shutdown(SdlApplication& app)
        {}

        void redraw()
        {
            redraw_ = true;
        }

        void clear_redraw()
        {
            redraw_ = false;
        }

        [[nodiscard]]
        bool should_redraw() const
        {
            return redraw_;
        }

    private:
        bool redraw_ = true;
    };
}
