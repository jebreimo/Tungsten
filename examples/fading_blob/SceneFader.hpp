//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Types.hpp>
#include <Xyz/Vector.hpp>

class SceneFader
{
public:
    explicit SceneFader(Tungsten::Size2I window_size);

    ~SceneFader();

    void set_window_size(Tungsten::Size2I window_size);

    void draw_previous_scene(float fadeout);

    void render_scene();
private:
    class SceneFaderImpl;
    std::unique_ptr<SceneFaderImpl> impl_;
};
