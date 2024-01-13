//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include <utility>
#include "EventMerger.hpp"
#include <SDL_events.h>

namespace Tungsten
{
    template <typename Event>
    class EventThrottler
    {
    public:
        explicit EventThrottler(uint64_t interval_msecs = 100,
                                EventMerger<Event> merger = {})
            : interval_msecs_(interval_msecs),
              merger_(std::move(merger))
        {}

        bool update(const Event& event)
        {
            if (time_ == 0)
            {
                event_ = event;
                time_ = event_.timestamp;
            }
            else if (!merger_.is_same_event(event))
            {
                return false;
            }
            accumulate(event);
            return true;
        }

        [[nodiscard]] bool is_due(uint32_t time) const
        {
            return time_ != 0 && time - time_ >= interval_msecs_;
        }

        [[nodiscard]] const Event& event(uint32_t time)
        {
            merger_.update(event_);
            event_.timestamp = time;
            return event_;
        }

        void clear()
        {
            time_ = 0;
        }

    private:
        Event event_ = {};
        uint64_t time_ = 0;
        uint64_t interval_msecs_ = 0;
        EventMerger<Event> merger_;
    };

    class EventThrottler2
    {
        explicit EventThrottler2(uint64_t interval_msecs,
                                 std::unique_ptr<EventMerger2> merger);

        bool update(const SDL_Event& event);

        [[nodiscard]] bool is_due(uint32_t time) const
        {
            return time_ != 0 && time - time_ >= interval_msecs_;
        }

        [[nodiscard]] const SDL_Event& event(uint32_t time);

        void clear();

    private:
        SDL_Event event_ = {};
        uint64_t time_ = 0;
        uint64_t interval_msecs_ = 0;
        std::unique_ptr<EventMerger2> merger_;
    };
}
