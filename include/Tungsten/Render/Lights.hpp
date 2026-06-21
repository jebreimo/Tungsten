//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Utilities.hpp>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    struct Light
    {
        Xyz::Vector3F ambient = {1.f, 1.f, 1.f};
        Xyz::Vector3F diffuse = {1.f, 1.f, 1.f};
        Xyz::Vector3F specular = {1.f, 1.f, 1.f};
    };

    struct DirectionalLight
    {
        Xyz::Vector3F direction = {1.0f, -1.0f, -10.0f};
        Light light;
    };

    struct LightAttenuation
    {
        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;
    };

    struct PointLight
    {
        Xyz::Vector3F position = {0.0f, 0.0f, 0.0f};
        Light light;
        LightAttenuation attenuation;
    };

    struct Spotlight
    {
        Xyz::Vector3F position = {0.0f, 0.0f, 0.0f};
        Xyz::Vector3F direction = {0.0f, 0.0f, -1.0f};
        Light light;
        LightAttenuation attenuation;
        float cut_off = Xyz::to_radians(12.5f);
        float outer_cut_off = Xyz::to_radians(15.0f);
    };
}
