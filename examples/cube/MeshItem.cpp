//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "MeshItem.hpp"

#include "Tungsten/ShaderPrograms/SmoothShader.hpp"

MeshItem::MeshItem(Tungsten::VertexArrayObject vao,
                   Tungsten::BufferHandle vertex_buffer,
                   Tungsten::BufferHandle element_buffer,
                   int32_t num_elements,
                   const Tungsten::ColorMaterial& material,
                   bool wireframe)
    : vao_(std::move(vao)),
      vertex_buffer_(std::move(vertex_buffer)),
      element_buffer_(std::move(element_buffer)),
      num_elements_(num_elements),
      material_(material),
      wireframe_(wireframe)
{
}

void MeshItem::set_model_matrix(const Xyz::Matrix4F& model_matrix)
{
    model_matrix_ = model_matrix;
}

void MeshItem::set_material(const Tungsten::ColorMaterial& material)
{
    material_ = material;
}

void MeshItem::set_texture(uint32_t texture_handle)
{
    texture_handle_ = texture_handle;
}

void MeshItem::draw(const Tungsten::Camera& camera,
                    Tungsten::SmoothShader& program) const
{
    program.set_model_view_matrix(camera.view_matrix() * model_matrix_);
    program.set_projection_matrix(camera.projection_matrix());
    program.material.set(material_);
    program.directional_light.set(
        Tungsten::DirectionalLight(
            {1.0f, 1.0f, -1.0f},
            {
                {1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f}
            }),
        camera.view_matrix()
    );
    vao_.bind();
    if (wireframe_)
        Tungsten::draw_line_elements_16(0, num_elements_);
    else
        Tungsten::draw_triangle_elements_16(0, num_elements_);
}
