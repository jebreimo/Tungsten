//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MultiGestureEventMerger.hpp"
#include <cmath>

namespace Tungsten
{
    bool MultiGestureEventMerger::can_update(const SDL_Event& event,
                                             const SDL_Event& new_event) const
    {
        return event.type == SDL_MULTIGESTURE
               && new_event.type == SDL_MULTIGESTURE
               && event.mgesture.numFingers == new_event.mgesture.numFingers;
    }

    void MultiGestureEventMerger::reset()
    {
        theta_ = 0;
        abs_theta_ = 0;
        dist_ = 0;
        abs_dist_ = 0;
    }

    void MultiGestureEventMerger::update(
        SDL_Event& event,
        const SDL_Event& new_event)
    {
        theta_ += new_event.mgesture.dTheta;
        abs_theta_ += std::abs(new_event.mgesture.dTheta);
        dist_ += new_event.mgesture.dDist;
        abs_dist_ += std::abs(new_event.mgesture.dDist);
        event = new_event;
        event.mgesture.dTheta = std::abs(theta_) > abs_theta_ * 0.5 ? theta_ : 0;
        event.mgesture.dDist = std::abs(dist_) > abs_dist_ * 0.5 ? dist_ : 0;
    }
}
