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
    MeshItem() = default;

    MeshItem(Tungsten::VertexArrayObject vao,
             Tungsten::BufferHandle vertex_buffer,
             Tungsten::BufferHandle element_buffer,
             int32_t num_elements,
             const Tungsten::ColorMaterial& material,
             bool wireframe = false);

    void set_model_matrix(const Xyz::Matrix4F& model_matrix);

    void set_material(const Tungsten::ColorMaterial& material);

    void set_texture(uint32_t texture_handle);

    /**
     * @brief Draws the mesh item using the given camera and shader program.
     *
     * @note The program must be already bound.
     */
    void draw(const Tungsten::Camera& camera,
              Tungsten::SmoothShader& program) const;

private:
    Xyz::Matrix4F model_matrix_ = Xyz::Matrix4F::identity();
    Tungsten::VertexArrayObject vao_;
    Tungsten::BufferHandle vertex_buffer_;
    Tungsten::BufferHandle element_buffer_;
    int32_t num_elements_;
    Tungsten::ColorMaterial material_;
    std::optional<int32_t> texture_handle_;
    bool wireframe_;
};
