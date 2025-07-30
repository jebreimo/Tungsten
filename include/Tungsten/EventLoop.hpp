//****************************************************************************
// Copyright © 2020 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2020-04-18.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL3/SDL_events.h>

namespace Tungsten
{
    class SdlApplication;

    class EventLoop
    {
    protected:
        explicit EventLoop(SdlApplication& application)
            : application_(&application)
        {}

    public:
        virtual ~EventLoop() = default;

        virtual bool on_event(const SDL_Event& event)
        {
            return false;
        }

        virtual void on_update()
        {}

        virtual void on_draw()
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

        [[nodiscard]]
        SdlApplication& application() const
        {
            return *application_;
        }
    private:
        bool redraw_ = true;
        SdlApplication* application_ = nullptr;
    };
}
