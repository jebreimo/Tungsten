//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/FixedStepClock.hpp"

#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    FixedStepClock::FixedStepClock(Duration step, uint32_t max_steps)
        : step_(step),
          max_steps_(max_steps)
    {
        if (step_.count() <= 0)
            TUNGSTEN_THROW("The step must be positive.");
    }

    FixedStepClock FixedStepClock::from_rate(uint32_t steps_per_second,
                                             uint32_t max_steps)
    {
        if (steps_per_second == 0)
            TUNGSTEN_THROW("The rate must be positive.");
        return FixedStepClock(Duration(std::chrono::seconds(1))
                              / steps_per_second,
                              max_steps);
    }

    uint32_t FixedStepClock::advance(Duration elapsed)
    {
        if (elapsed.count() > 0)
            accumulated_ += elapsed;

        const auto due = accumulated_ / step_;
        if (due > max_steps_)
        {
            accumulated_ = Duration(0);
            return max_steps_;
        }

        accumulated_ -= due * step_;
        return uint32_t(due);
    }

    FixedStepClock::Duration FixedStepClock::step() const
    {
        return step_;
    }

    float FixedStepClock::step_seconds() const
    {
        return std::chrono::duration<float>(step_).count();
    }

    float FixedStepClock::alpha() const
    {
        return float(double(accumulated_.count()) / double(step_.count()));
    }

    void FixedStepClock::reset()
    {
        accumulated_ = Duration(0);
    }
}
