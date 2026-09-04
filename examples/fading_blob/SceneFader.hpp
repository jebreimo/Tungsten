//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <Tungsten/Gl/GlTypes.hpp>

class SceneFader
{
public:
    explicit SceneFader(Tungsten::Size2I window_size);

    ~SceneFader();

    void set_window_size(Tungsten::Size2I window_size);

    /**
     * @brief Copies the previous frame's scene, darkened by @a fade_step.
     *
     * @param fade_step the amount subtracted from each color channel,
     *      where 1.0 is the full range from black to white.
     */
    void draw_previous_scene(float fade_step);

    void render_scene();
private:
    class SceneFaderImpl;
    std::unique_ptr<SceneFaderImpl> impl_;
};
