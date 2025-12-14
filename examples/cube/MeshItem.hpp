//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>

class MeshItem
{
public:
    MeshItem(Tungsten::VertexArrayObject vao,
             const Tungsten::Material& material);

    void set_model_matrix(const Xyz::Matrix4F& model_matrix);

    void set_material(const Tungsten::Material& material);

    void draw(const Tungsten::Camera& camera,
              Tungsten::SmoothMeshShader& program) const;

private:
    Xyz::Matrix4F model_matrix_ = Xyz::Matrix4F::identity();
    Tungsten::VertexArrayObject vao_;
    Tungsten::Material material_;
};
