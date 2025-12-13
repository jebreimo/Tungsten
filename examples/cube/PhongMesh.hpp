//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>

struct Material
{
    Xyz::Vector3F diffuse_albedo = {1, 1, 1};
    Xyz::Vector3F specular_albedo = {1, 1, 1};
    float specular_exponent = 32.0f;
};

class PhongMesh
{
public:
    void set_light_pos(const Xyz::Vector3F& pos);

    void set_material(const Material& material);

    void setup();

    void draw(const Tungsten::Camera& camera);
private:
    Xyz::Matrix4F model_matrix_ = Xyz::Matrix4F::identity();
    Xyz::Vector3F light_pos_ = {0, 0, 0};
    Material material_;
    std::shared_ptr<Tungsten::VertexArrayHandle> vao_;
    std::shared_ptr<Tungsten::ProgramHandle> program_;
    Tungsten::BufferHandle vertex_buffer_;
    Tungsten::BufferHandle indexes_buffer_;
};
