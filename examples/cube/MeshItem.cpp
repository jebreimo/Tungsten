//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MeshItem.hpp"

MeshItem::MeshItem(Tungsten::VertexArrayObject vao,
                   const Tungsten::Material& material)
    : vao_(std::move(vao)),
      material_(material)
{
}

void MeshItem::set_model_matrix(const Xyz::Matrix4F& model_matrix)
{
    model_matrix_ = model_matrix;
}

void MeshItem::set_material(const Tungsten::Material& material)
{
    material_ = material;
}

void MeshItem::draw(const Tungsten::Camera& camera,
                    Tungsten::SmoothMeshShader& program) const
{
    program.set_model_view_matrix(camera.view * model_matrix_);
    program.set_material(material_);
    vao_.bind();
    Tungsten::draw_triangle_elements_16(0, vao_.element_count);
}
