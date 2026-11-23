//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <chrono>
#include <cstdint>

namespace Tungsten
{
    /**
     * @brief Divides real elapsed time into simulation steps of a fixed
     *      duration.
     *
     * Each frame, pass the time that has elapsed to advance() and run the
     * simulation the number of steps it returns. The time left over is
     * carried to the next frame, so the simulation keeps pace with real
     * time regardless of the frame rate, and alpha() tells how far the
     * current moment is between the last step and the next one, for
     * interpolating what is drawn.
     */
    class FixedStepClock
    {
    public:
        using Duration = std::chrono::nanoseconds;

        /**
         * @param step The simulated time of one step. Must be positive.
         * @param max_steps The most steps a single call to advance() will
         *      return. Time beyond that is dropped, so the simulation slows
         *      down rather than falling ever further behind when a step
         *      costs more than it simulates, or after a long pause.
         */
        explicit FixedStepClock(Duration step, uint32_t max_steps = 8);

        /**
         * Creates a clock with @a steps_per_second steps per second.
         */
        static FixedStepClock from_rate(uint32_t steps_per_second,
                                        uint32_t max_steps = 8);

        /**
         * Adds @a elapsed to the accumulated time and returns the number
         * of whole steps that are now due.
         */
        uint32_t advance(Duration elapsed);

        /** The duration of one step. */
        [[nodiscard]]
        Duration step() const;

        /** The duration of one step, in seconds. */
        [[nodiscard]]
        float step_seconds() const;

        /**
         * The accumulated time that hasn't become a step yet, as a
         * fraction of a step in [0, 1).
         */
        [[nodiscard]]
        float alpha() const;

        /** Discards the accumulated time. */
        void reset();

    private:
        Duration step_;
        Duration accumulated_{0};
        uint32_t max_steps_;
    };
}
