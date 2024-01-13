//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "EventThrottler.hpp"

namespace Tungsten
{
    EventThrottler2::EventThrottler2(uint64_t interval_msecs, std::unique_ptr<EventMerger2> merger)
        : interval_msecs_(interval_msecs),
          merger_(std::move(merger))
    {}

    bool EventThrottler2::update(const SDL_Event& event)
    {
        if (time_ == 0)
        {
            memcpy(&event_, &event, sizeof(event));
            time_ = event.common.timestamp;
        }
        else if (!merger_->can_update(event_, event))
        {
            return false;
        }
        merger_->update(event_, event);
        return true;
    }

    const SDL_Event& EventThrottler2::event(uint32_t time)
    {
        update(event_);
        event_.common.timestamp = time;
        return event_;
    }

    void EventThrottler2::clear()
    {
        time_ = 0;
    }
}
