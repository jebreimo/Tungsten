//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MeshItem.hpp"

#include "Tungsten/ShaderPrograms/TexturedSmoothMeshShader.hpp"

MeshItem::MeshItem(Tungsten::VertexArrayObject vao,
                   const Tungsten::Material& material,
                   bool wireframe)
    : vao_(std::move(vao)),
      material_(material),
      wireframe_(wireframe)
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

void MeshItem::set_texture(uint32_t texture_handle)
{
    texture_handle_ = texture_handle;
}

void MeshItem::draw(const Tungsten::Camera& camera,
                    Tungsten::SmoothMeshShader& program) const
{
    program.set_model_view_matrix(camera.view * model_matrix_);
    program.set_material(material_);
    if (texture_handle_)
    {
        if (auto* tex_prog = dynamic_cast<Tungsten::TexturedSmoothMeshShader*>(&program))
            tex_prog->set_texture(static_cast<int32_t>(*texture_handle_));
    }
    vao_.bind();
    if (wireframe_)
        Tungsten::draw_line_elements_16(0, vao_.element_count);
    else
        Tungsten::draw_triangle_elements_16(0, vao_.element_count);
}
