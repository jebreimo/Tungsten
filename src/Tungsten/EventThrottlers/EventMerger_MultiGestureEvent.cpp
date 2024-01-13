//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "EventMerger_MultiGestureEvent.hpp"
#include <cmath>

namespace Tungsten
{
    bool EventMerger<SDL_MultiGestureEvent>::can_update(
        const SDL_MultiGestureEvent& event,
        const SDL_MultiGestureEvent& other) const
    {
        return event.numFingers == other.numFingers;
    }

    void EventMerger<SDL_MultiGestureEvent>::reset()
    {
        theta = 0;
        abs_theta = 0;
        dist = 0;
        abs_dist = 0;
    }

    void EventMerger<SDL_MultiGestureEvent>::update(
        SDL_MultiGestureEvent& event,
        const SDL_MultiGestureEvent& other)
    {
        theta += other.dTheta;
        abs_theta += std::abs(other.dTheta);
        dist += other.dDist;
        abs_dist += std::abs(other.dDist);
        event = other;
        event.dTheta = std::abs(theta) > abs_theta * 0.5 ? theta : 0;
        event.dDist = std::abs(dist) > abs_dist * 0.5 ? dist : 0;
    }

    bool MultiGestureEventMerger::can_update(const SDL_Event& event,
                                             const SDL_Event& other) const
    {
        return event.type == SDL_MULTIGESTURE && other.type == SDL_MULTIGESTURE
               && event.mgesture.numFingers
                  == other.mgesture.numFingers;
    }
}
