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
    EventThrottler::EventThrottler(std::unique_ptr<EventMerger> merger,
                                   uint32_t interval_msecs)
        : interval_msecs_(interval_msecs),
          merger_(std::move(merger))
    {}

    bool EventThrottler::update(const SDL_Event& event)
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

    bool EventThrottler::is_due(uint32_t time) const
    {
        return time_ != 0 && time - time_ >= interval_msecs_;
    }

    const SDL_Event& EventThrottler::event(uint32_t time)
    {
        event_.common.timestamp = time;
        return event_;
    }

    void EventThrottler::clear()
    {
        time_ = 0;
        merger_->reset();
    }
}
