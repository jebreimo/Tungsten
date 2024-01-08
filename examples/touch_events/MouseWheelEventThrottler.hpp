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

class MouseWheelEventThrottler
{
public:
    explicit MouseWheelEventThrottler(uint64_t interval_msecs = 100);

    bool update(const SDL_MouseWheelEvent& event);

    [[nodiscard]] bool has_event(uint32_t time) const;

    [[nodiscard]] SDL_MouseWheelEvent event() const;

    void clear();

private:
    SDL_MouseWheelEvent event_ = {};
    float x_ = 0;
    float y_ = 0;
    uint64_t time_ = 0;
    uint64_t interval_msecs_ = 0;
};
