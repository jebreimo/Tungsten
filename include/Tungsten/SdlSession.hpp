//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2016-02-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <string>
#include "Tungsten/SdlConfiguration.hpp"

namespace Tungsten
{
    class SdlSession
    {
    public:
        explicit SdlSession(uint32_t flags);

        SdlSession(const SdlSession& other) = delete;

        SdlSession(SdlSession&& other) noexcept;

        ~SdlSession();

        SdlSession& operator=(const SdlSession& other) = delete;

        SdlSession& operator=(SdlSession&& other) noexcept;

        [[nodiscard]]
        bool is_active() const;

        [[nodiscard]]
        bool touch_events_enabled() const;

        void set_touch_events_enabled(bool enabled);
    private:
        int active_;
    };
}
