//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Matrix.hpp>

#include "Tungsten/ArrayBuffer.hpp"

class Shape2D
{
public:
    Shape2D() = default;

    Shape2D(Tungsten::VertexArray<Xyz::Vector2F> vertex_array,
            const Xyz::Vector4F& color);

    [[nodiscard]] const Xyz::Vector4F& color() const;

    void set_color(const Xyz::Vector4F& color);

    const Tungsten::VertexArray<Xyz::Vector2F>& vertex_array() const;
private:
    Tungsten::VertexArray<Xyz::Vector2F> vertex_array_;
    Xyz::Vector4F color_;
};

class Shape2DRenderer
{
public:
    using Buffer = Tungsten::ArrayBuffer<Xyz::Vector2F>;

    Shape2DRenderer();

    ~Shape2DRenderer();

    Shape2DRenderer(const Shape2DRenderer&) = delete;

    Shape2DRenderer(Shape2DRenderer&&) noexcept;

    Shape2DRenderer& operator=(const Shape2DRenderer&) = delete;

    Shape2DRenderer& operator=(Shape2DRenderer&&) noexcept;

    Shape2D create_shape(const Buffer& buffer,
                         const Xyz::Vector4F& color = {1, 1, 1, 1}) const;

    void set_model_view_matrix(const Xyz::Matrix3F& matrix);

    void set_projection_matrix(const Xyz::Matrix3F& matrix);

    void draw(const Shape2D& shape);
private:
    class Basic2DProgram;
    std::unique_ptr<Basic2DProgram> program_;
    Xyz::Matrix3F model_view_matrix_;
    Xyz::Matrix3F projection_matrix_;
};
