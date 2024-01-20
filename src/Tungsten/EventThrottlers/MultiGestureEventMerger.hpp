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
    class MultiGestureEventMerger : public EventMerger
    {
    public:
        [[nodiscard]]
        bool can_update(const SDL_Event& event,
                        const SDL_Event& new_event) const override;

        void reset() override;

        void update(SDL_Event& event, const SDL_Event& new_event) override;
    private:
        float theta_ = 0;
        float abs_theta_ = 0;
        float dist_ = 0;
        float abs_dist_ = 0;
    };
}
