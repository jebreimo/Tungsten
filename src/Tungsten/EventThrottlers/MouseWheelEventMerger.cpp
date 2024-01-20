//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MouseWheelEventMerger.hpp"

namespace Tungsten
{
    bool MouseWheelEventMerger::can_update(const SDL_Event& old_event,
                                           const SDL_Event& new_event) const
    {
        return old_event.type == new_event.type
               && old_event.wheel.windowID == new_event.wheel.windowID;
    }

    void MouseWheelEventMerger::update(SDL_Event& old_event,
                                       const SDL_Event& new_event)
    {
        old_event.wheel.preciseX += new_event.wheel.preciseX;
        old_event.wheel.preciseY += new_event.wheel.preciseY;
    }

    void MouseWheelEventMerger::reset()
    {}
}
