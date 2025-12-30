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
             const Tungsten::Material& material,
             bool wireframe = false);

    void set_model_matrix(const Xyz::Matrix4F& model_matrix);

    void set_material(const Tungsten::Material& material);

    void set_texture(uint32_t texture_handle);

    /**
     * @brief Draws the mesh item using the given camera and shader program.
     *
     * @note The program must be already bound.
     */
    void draw(const Tungsten::Camera& camera,
              Tungsten::SmoothMeshShader& program) const;

private:
    Xyz::Matrix4F model_matrix_ = Xyz::Matrix4F::identity();
    Tungsten::VertexArrayObject vao_;
    Tungsten::Material material_;
    std::optional<int32_t> texture_handle_;
    bool wireframe_;
};
