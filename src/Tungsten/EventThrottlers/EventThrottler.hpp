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
    class EventThrottler
    {
    public:
        EventThrottler() noexcept = default;

        EventThrottler(EventThrottler&&) noexcept = default;

        explicit EventThrottler(std::unique_ptr<EventMerger> merger,
                                uint32_t interval_msecs = 100);

        EventThrottler& operator=(EventThrottler&&) noexcept = default;

        bool update(const SDL_Event& event);

        [[nodiscard]] bool is_due(uint32_t time) const;

        [[nodiscard]] const SDL_Event& event(uint32_t time);

        void clear();

    private:
        SDL_Event event_ = {};
        uint32_t time_ = 0;
        uint32_t interval_msecs_ = 0;
        std::unique_ptr<EventMerger> merger_;
    };
}
