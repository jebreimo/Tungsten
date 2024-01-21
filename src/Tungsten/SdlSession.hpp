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
#include <SDL2/SDL.h>
#include "Tungsten/SdlConfiguration.hpp"

namespace Tungsten
{
    class SdlSession
    {
    public:
        explicit SdlSession(uint32_t flags);

        SdlSession(SdlSession&& other) noexcept;

        ~SdlSession();

        SdlSession& operator=(SdlSession&& other) noexcept;

        [[nodiscard]]
        bool is_active() const;
    private:
        int active_;
    };

    void configure_sdl(const SdlSession& session,
                       const SdlConfiguration& configuration);

    uint32_t get_sdl_init_flags(const SdlConfiguration& configuration);
}
