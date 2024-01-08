//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MouseWheelEventThrottler.hpp"

MouseWheelEventThrottler::MouseWheelEventThrottler(uint64_t interval_msecs)
    : interval_msecs_(interval_msecs)
{
}

bool MouseWheelEventThrottler::update(const SDL_MouseWheelEvent& event)
{
    if (time_ == 0)
    {
        event_ = event;
        time_ = event_.timestamp;
    }
    x_ += event.preciseX;
    y_ += event.preciseY;
    return true;
}

bool MouseWheelEventThrottler::has_event(uint32_t time) const
{
    return time_ != 0 && time - time_ >= interval_msecs_;
}

SDL_MouseWheelEvent MouseWheelEventThrottler::event() const
{
    SDL_MouseWheelEvent event = event_;
    event.preciseX = x_;
    event.preciseY = y_;
    return event;
}

void MouseWheelEventThrottler::clear()
{
    x_ = 0;
    y_ = 0;
    time_ = 0;
}
