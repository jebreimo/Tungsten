//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-13.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "EventMerger.hpp"

namespace Tungsten
{
    class MouseWheelEventMerger : public EventMerger
    {
    public:
        [[nodiscard]]
        bool can_update(const SDL_Event& old_event,
                        const SDL_Event& new_event) const override;

        void update(SDL_Event& old_event,
                    const SDL_Event& new_event) override;

        void reset() override;
    };
}
