//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL_events.h>
#include "EventMerger.hpp"

namespace Tungsten
{
    template <>
    class EventMerger<SDL_MultiGestureEvent>
    {
    public:
        [[nodiscard]]
        bool can_update(const SDL_MultiGestureEvent& event,
                        const SDL_MultiGestureEvent& other) const;

        void reset();

        void update(SDL_MultiGestureEvent& event,
                    const SDL_MultiGestureEvent& other);
    private:
        float theta = 0;
        float abs_theta = 0;
        float dist = 0;
        float abs_dist = 0;
    };

    class MultiGestureEventMerger : public EventMerger2
    {
    public:
        [[nodiscard]]
        bool can_update(const SDL_Event& event,
                        const SDL_Event& other) const override;

        void reset() override;

        void update(SDL_Event& event, const SDL_Event& other) override;
    };
}
