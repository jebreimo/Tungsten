//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-01-05.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include <cstdint>
#include <SDL_events.h>

class AccumulatedValue
{
public:
    void add(float value);

    [[nodiscard]] float get(float min_abs_ratio = 0.5) const;

    void clear();
private:
    float value_ = 0;
    float abs_value_ = 0;
};

class MultiGestureEventThrottler
{
public:
    explicit MultiGestureEventThrottler(uint64_t interval_msecs = 100);

    bool update(const SDL_MultiGestureEvent& event);

    [[nodiscard]] bool has_event(uint32_t time) const;

    [[nodiscard]] SDL_MultiGestureEvent event() const;

    void clear();
private:
    SDL_MultiGestureEvent event_ = {};
    AccumulatedValue dist_;
    AccumulatedValue theta_;
    uint64_t time_ = 0;
    uint64_t interval_msecs_ = 0;
};
