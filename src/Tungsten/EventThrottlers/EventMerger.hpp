//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

namespace Tungsten
{

    template <typename Event>
    struct EventMerger
    {
        bool can_update(const Event& event, const Event& other) const;

        void reset();

        void update(Event& event, const Event& other);
    };

    class EventMerger2
    {
    public:
        [[nodiscard]]
        virtual bool can_update(const SDL_Event& event,
                                const SDL_Event& other) const = 0;

        virtual void reset() = 0;

        virtual void update(SDL_Event& event, const SDL_Event& other) = 0;
    };
}
