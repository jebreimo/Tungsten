//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MultiGestureEventThrottler.hpp"
#include <cmath>

void AccumulatedValue::add(float value)
{
    value_ += value;
    abs_value_ += std::abs(value);
}

float AccumulatedValue::get(float min_abs_ratio) const
{
    return value_ > abs_value_ * min_abs_ratio ? value_ : 0;
}

void AccumulatedValue::clear()
{
    value_ = 0;
    abs_value_ = 0;
}

MultiGestureEventThrottler::MultiGestureEventThrottler(uint64_t interval_msecs)
    : interval_msecs_(interval_msecs)
{}

bool MultiGestureEventThrottler::update(const SDL_MultiGestureEvent& event)
{
    if (time_ == 0)
    {
        event_ = event;
        time_ = event_.timestamp;
    }
    else if (event.numFingers != event_.numFingers)
    {
        return false;
    }
    dist_.add(event.dDist);
    theta_.add(event.dTheta);
    return true;
}

bool MultiGestureEventThrottler::has_event(uint32_t time) const
{
    return time_ != 0 && time - time_ >= interval_msecs_;
}

SDL_MultiGestureEvent MultiGestureEventThrottler::event() const
{
    SDL_MultiGestureEvent event = event_;
    event.dTheta = theta_.get();
    event.dDist = dist_.get();
    return event;
}

void MultiGestureEventThrottler::clear()
{
    dist_.clear();
    theta_.clear();
    time_ = 0;
}
