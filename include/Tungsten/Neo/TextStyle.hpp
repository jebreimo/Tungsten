//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <utility>
#include <Xyz/Vector.hpp>
#include "Tungsten/Render/Font.hpp"

namespace Tungsten
{
    /**
     * The appearance of a piece of text: which font draws it, in what colour,
     * how its lines line up with each other, and which point of it sits on its
     * node's origin.
     *
     * A style is immutable and shared — TextComponents hold
     * `std::shared_ptr<const TextStyle>` — so a table's hundred value cells
     * name one style object instead of each carrying its own copy of these
     * fields. Restyling all of them is a matter of pointing them at another
     * style; there is no per-item state to keep in step.
     *
     * Sharing is also what makes change detection cheap: TextSystem compares
     * the style *pointer*, not its contents.
     *
     * The fields split in two along the line TextSystem cares about:
     *
     * - `font`, `line_gap` and `horizontal_alignment` determine the glyph
     *   geometry, and the two anchors determine where that geometry sits
     *   relative to the node origin. All five are baked into the vertex data.
     * - `color`, together with the font's atlas texture, determines the
     *   Material. It never touches the vertex data.
     *
     * The split is per style *object*, not per field, so pointing a component
     * at a different style re-tessellates it even if only the colour differs.
     * That is the right default — styles are set up once — but it makes
     * TextStyle the wrong place for a colour that changes at runtime. Use
     * TextComponent::color_override for that: it only re-resolves the
     * material.
     *
     * Everything about *where* the text goes, beyond the anchor, belongs to
     * the node: position, rotation and scale come from its transform, so
     * moving text costs nothing.
     */
    struct TextStyle
    {
        std::shared_ptr<const Font> font;
        Xyz::Vector4F color = {0.f, 0.f, 0.f, 1.f};
        /**
         * Space between consecutive baselines beyond the font's own line
         * height, as a fraction of that height. 0 sets the lines solid.
         */
        float line_gap = 0.1f;
        HorizontalAlignment horizontal_alignment = HorizontalAlignment::LEFT;
        HorizontalAnchor horizontal_anchor = HorizontalAnchor::LEFT;
        VerticalAnchor vertical_anchor = VerticalAnchor::TOP;
    };

    /**
     * Interns a style as the shared, immutable object a TextComponent holds —
     * the spelling that keeps the const in `shared_ptr<const TextStyle>` from
     * having to be written at every call site.
     */
    [[nodiscard]]
    inline std::shared_ptr<const TextStyle> make_text_style(TextStyle style)
    {
        return std::make_shared<const TextStyle>(std::move(style));
    }
} // Tungsten
