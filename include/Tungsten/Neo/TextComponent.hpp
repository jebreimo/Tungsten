//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <Xyz/BBox.hpp>
#include <Xyz/Vector.hpp>
#include "ResourceRefs.hpp"

namespace Tungsten
{
    struct TextStyle;

    /**
     * Makes a node display a line — or several lines — of text. It is the
     * *source* form of a drawable, not a drawable itself: TextSystem::update()
     * compiles it into a Mesh and a Material and publishes those through a
     * RenderableComponent on the same node, which is what SnapshotBuilder and
     * the Renderer then see. Neither of them knows anything about text.
     *
     * So a text node carries two components — this one, which the application
     * writes, and a RenderableComponent, which TextSystem owns and overwrites.
     * Do not edit the latter by hand; the next update() would undo it.
     */
    struct TextComponent
    {
        /**
         * The text to draw, UTF-8, with '\n' separating lines. An empty text
         * draws nothing and holds no GPU memory.
         */
        std::string text;
        /**
         * The shared style. A component without one (or whose style has no
         * font) draws nothing.
         */
        std::shared_ptr<const TextStyle> style;
        /**
         * A colour for this item alone, in place of the style's. Setting it
         * only re-resolves the material — the glyphs are not re-tessellated —
         * so this, not a swapped-in style, is the way to recolour text that
         * changes while it is on screen.
         *
         * TextSystem interns one material per distinct colour and never
         * retires them, on the assumption that an application uses few. Do not
         * drive this from a continuous value — a colour animated per frame
         * would mint a material per frame.
         */
        std::optional<Xyz::Vector4F> color_override;
        bool visible = true;
        /**
         * The render layer of the published RenderableComponent, which forms
         * the top bits of the sort key. Text is drawn in the transparent pass;
         * a higher layer is how a label is kept on top of other transparent
         * geometry.
         */
        uint32_t render_layer = 0;

        /**
         * What TextSystem last compiled this component into: its own copy of
         * the inputs above, plus the resources they produced. The diff against
         * the fields above is what takes the place of dirty flags.
         *
         * Only TextSystem can write it — the shadow copies are what the diff
         * compares against, so an application that edited them would make the
         * system miss a change. It lives here, rather than in a side table
         * keyed by node, so that a component and the record of what it built
         * stay together when the scene's swap-and-pop removal moves it.
         *
         * The accessors are for tests and for anything that wants to observe
         * what a text item resolved to; there is nothing to set.
         */
        class BuiltState
        {
        public:
            /**
             * `slot`'s value before this component has ever been built.
             */
            static constexpr uint32_t NO_SLOT = UINT32_MAX;

            /**
             * The mesh holding this text's glyph quads. The ref stays the same
             * across rebuilds — only the slices inside it are reallocated — so
             * the published RenderableComponent rarely has to be re-pointed.
             * Null while the text is empty, which is also when the item owns
             * no GPU memory at all.
             */
            [[nodiscard]]
            MeshRef mesh() const
            {
                return mesh_;
            }

            /**
             * The material pairing the font's atlas with the effective colour.
             * Shared with every other item that resolves to the same pair.
             */
            [[nodiscard]]
            MaterialRef material() const
            {
                return material_;
            }

            /**
             * The tessellated text's extent in the node's local space, handed
             * to the RenderableComponent so text is frustum-culled like any
             * other geometry. Flat in z.
             */
            [[nodiscard]]
            const Xyz::BBox3F& bounds() const
            {
                return bounds_;
            }

        private:
            friend class TextSystem;

            std::string text_;
            std::shared_ptr<const TextStyle> style_;
            std::optional<Xyz::Vector4F> color_override_;
            bool visible_ = false;
            uint32_t render_layer_ = 0;

            MeshRef mesh_;
            MaterialRef material_;
            Xyz::BBox3F bounds_;
            /**
             * Index of this component's entry in TextSystem's slot table, or
             * NO_SLOT. The table is how the system reclaims meshes whose
             * component — or whose whole node — has been removed: after each
             * sweep, an entry no live component claims is released.
             */
            uint32_t slot_ = NO_SLOT;
        };

        BuiltState built;
    };
} // Tungsten
