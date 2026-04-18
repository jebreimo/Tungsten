//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-04-08.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <Xyz/Vector.hpp>

#include "Font.hpp"

namespace Tungsten
{
    enum class HorizontalAnchor : uint8_t
    {
        LEFT,
        CENTER,
        RIGHT
    };

    enum class VerticalAnchor : uint8_t
    {
        TOP,
        CENTER,
        BOTTOM,
        BASELINE
    };

    enum class HorizontalAlignment : uint8_t
    {
        LEFT,
        CENTER,
        RIGHT
    };

    struct TextStyleData
    {
        std::shared_ptr<Font> font;
        float rotation = 0;
        float line_gap = 0.5;
        Xyz::Vector4F color = {0.f, 0.f, 0.f, 1.f};
        HorizontalAlignment horizontal_alignment = HorizontalAlignment::LEFT;
        HorizontalAnchor horizontal_anchor = HorizontalAnchor::LEFT;
        VerticalAnchor vertical_anchor = VerticalAnchor::TOP;
    };
} // Tungsten
