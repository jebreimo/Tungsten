//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <SDL_events.h>

namespace Tungsten
{
    class EventMerger
    {
    public:
        virtual ~EventMerger() = default;

        [[nodiscard]]
        virtual bool can_update(const SDL_Event& event,
                                const SDL_Event& new_event) const = 0;

        virtual void update(SDL_Event& event, const SDL_Event& new_event) = 0;

        virtual void reset() = 0;
    };
}
