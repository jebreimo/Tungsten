//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-03-14.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Camera.hpp"

namespace Tungsten
{
    class Renderable : std::enable_shared_from_this<Renderable>
    {
    public:
        virtual ~Renderable() = default;

        virtual void prepare(const Camera& camera) = 0;

        virtual void render(const Camera& camera) const = 0;
    };
} // Tungsten
