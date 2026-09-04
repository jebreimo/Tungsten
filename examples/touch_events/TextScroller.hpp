//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "RingBuffer.hpp"

namespace Tungsten
{
    constexpr Xyz::Vector4F TEXT_COLOR = {1.f, 1.f, 1.f, 1.f};

    // Space between consecutive lines, in pixels — the camera below makes one
    // world unit one pixel.
    constexpr float LINE_GAP = 2.0f;

    // How many lines are kept. The oldest is dropped, node and all, when a new
    // one arrives on a full buffer.
    constexpr size_t MAX_LINES = 100;

    constexpr Transform at(float x, float y, float z = 0.0f)
    {
        return Transform{.translation = {x, y, z}};
    }

    class TextScroller
    {
    public:
        TextScroller(Scene& scene, const std::shared_ptr<const Font>& font)
            : scene_(&scene),
              // One immutable style shared by every line, so all of them also
              // share one material and are bound once for the whole log.
              style_(make_text_style({
                  .font = font,
                  .color = TEXT_COLOR,
                  .horizontal_alignment = HorizontalAlignment::LEFT,
                  .horizontal_anchor = HorizontalAnchor::LEFT,
                  .vertical_anchor = VerticalAnchor::BOTTOM
              }))
        {}

        void add_text(std::string text)
        {
            // push_back drops the oldest entry when the buffer is full, so its
            // node has to leave the scene at the same time. TextSystem
            // reclaims the glyphs on its next sweep.
            if (nodes_.full())
                nodes_.front().remove();

            const auto node = scene_->add_node();
            node.add(TextComponent{.text = std::move(text), .style = style_});
            nodes_.push_back(node);
        }

        // Stacks the lines upwards from the bottom-left corner, newest first.
        // Must run after TextSystem::update(), which is what fills in the
        // bounds this reads, and before Scene::resolve_transforms().
        void layout(const Viewport& viewport) const
        {
            const float left = -viewport.size[0] / 2.0f;
            float y = -viewport.size[1] / 2.0f;

            for (size_t i = nodes_.size(); i-- > 0;)
            {
                const NodeHandle& node = nodes_[i];
                node.set_local_transform(at(left, y));
                const auto& bounds = node.get<TextComponent>().built.bounds();
                y += bounds.max[1] - bounds.min[1] + LINE_GAP;
            }
        }

    private:
        Scene* scene_;
        std::shared_ptr<const TextStyle> style_;
        Chorasmia::RingBuffer<NodeHandle, MAX_LINES> nodes_;
    };
}
