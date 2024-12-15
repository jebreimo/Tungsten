//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>
#include "Tungsten/GlFrameBuffers.hpp"
#include "Tungsten/GlTextures.hpp"
#include "Tungsten/VertexArray.hpp"

class SceneFader
{
public:
    explicit SceneFader(std::pair<int, int> window_size);

    ~SceneFader();

    void set_window_size(std::pair<int, int> window_size);

    void set_fadeout(float fadeout);

    void prepare_scene();

    void render_scene();
private:
    class SceneFaderImpl;
    std::unique_ptr<SceneFaderImpl> impl_;
};
