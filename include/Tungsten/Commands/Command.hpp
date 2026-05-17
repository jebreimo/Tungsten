//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Camera.hpp"

namespace Tungsten
{
    class Command
    {
    public:
        Command() = default;

        virtual ~Command() = default;

        virtual void run(const Camera& camera) = 0;
    };
} // Tungsten
