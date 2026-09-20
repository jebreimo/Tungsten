//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <Tungsten/Gpu/GpuTypes.hpp>
#include <Tungsten/Rendering/RenderPassDescriptor.hpp>
#include <Tungsten/Viewport.hpp>

namespace Tungsten
{
    class Renderer;
    class ResourceManager;
}

/**
 * A motion trail, drawn entirely through the portable path.
 *
 * Two colour textures are ping-ponged. Each frame the fader copies the one
 * drawn last frame into the other, darkened, and the scene is then drawn on
 * top of that faded copy; the result is presented to the window and the two
 * swap roles. Nothing here touches OpenGL: the accumulation buffers are
 * RenderTargets, the full-screen quad is an ordinary mesh drawn through an
 * ordinary pipeline, and the fade amount reaches the shader in a material's
 * parameter block.
 */
class SceneFader
{
public:
    SceneFader(Tungsten::ResourceManager& resources,
               Tungsten::Size2I window_size);

    ~SceneFader();

    SceneFader(const SceneFader&) = delete;
    SceneFader& operator=(const SceneFader&) = delete;

    void set_window_size(Tungsten::Size2I window_size);

    /**
     * @brief Copies the previous frame's scene into the accumulation buffer,
     *      darkened by @a fade_step.
     *
     * @param fade_step the amount subtracted from each color channel,
     *      where 1.0 is the full range from black to white.
     */
    void draw_previous_scene(Tungsten::Renderer& renderer, float fade_step);

    /**
     * @brief The pass a scene should be drawn in to land on top of the trail.
     *
     * Its colour attachment loads rather than clears — the faded copy
     * underneath *is* the trail.
     */
    [[nodiscard]]
    Tungsten::RenderPassDescriptor accumulation_pass(
        const Tungsten::Viewport& viewport) const;

    /** @brief Presents the accumulation buffer and swaps the two. */
    void render_scene(Tungsten::Renderer& renderer,
                      const Tungsten::Viewport& viewport);

private:
    class SceneFaderImpl;
    std::unique_ptr<SceneFaderImpl> impl_;
};
