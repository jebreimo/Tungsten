//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Shape2D.hpp"

#include "Tungsten/GlVertices.hpp"
#include "Tungsten/ShaderProgramBuilder.hpp"
#include "Tungsten/VertexArrayBuilder.hpp"

class Shape2DRenderer::Basic2DProgram
{
public:
    Basic2DProgram()
    {
        program = Tungsten::ShaderProgramBuilder()
            .add_shader(Tungsten::ShaderType::VERTEX, R"(
                #version 100
                uniform highp mat3 u_model_view_matrix;
                uniform highp mat3 u_projection_matrix;
                uniform highp float u_z;
                attribute vec2 a_position;
                void main()
                {
                    vec3 pos = u_projection_matrix * u_model_view_matrix * vec3(a_position, 1.0);
                    gl_Position = vec4(vec2(pos), u_z, 1.0);
                })")
            .add_shader(Tungsten::ShaderType::FRAGMENT, R"(
                #version 100
                uniform highp vec4 u_color;
                void main()
                {
                    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
                })")
            .build();

        position_attr = Tungsten::get_vertex_attribute(program, "a_position");
        model_view_matrix = Tungsten::get_uniform<Xyz::Matrix3F>(program, "u_model_view_matrix");
        projection_matrix = Tungsten::get_uniform<Xyz::Matrix3F>(program, "u_projection_matrix");
        color = Tungsten::get_uniform<Xyz::Vector4F>(program, "u_color");
        z = Tungsten::get_uniform<float>(program, "u_z");
    }

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix3F> model_view_matrix;
    Tungsten::Uniform<Xyz::Matrix3F> projection_matrix;
    Tungsten::Uniform<Xyz::Vector4F> color;
    Tungsten::Uniform<float> z;
    Tungsten::Uniform<float> point_size;

    uint32_t position_attr;
};

Shape2D::Shape2D(Tungsten::VertexArray<Xyz::Vector2F> vertex_array,
                 const Xyz::Vector4F& color)
    : vertex_array_(std::move(vertex_array)),
      color_(color)
{
}

const Xyz::Vector4F& Shape2D::color() const
{
    return color_;
}

void Shape2D::set_color(const Xyz::Vector4F& color)
{
    color_ = color;
}

const Tungsten::VertexArray<Xyz::Vector2F>& Shape2D::vertex_array() const
{
    return vertex_array_;
}

Shape2DRenderer::Shape2DRenderer()
    : program_(std::make_unique<Basic2DProgram>()),
      model_view_matrix_(Xyz::Matrix3F::identity()),
      projection_matrix_(Xyz::Matrix3F::identity())
{
}

Shape2DRenderer::~Shape2DRenderer() = default;

Shape2DRenderer::Shape2DRenderer(Shape2DRenderer&& rhs) noexcept
    : program_(std::move(rhs.program_))
{
}

Shape2DRenderer& Shape2DRenderer::operator=(Shape2DRenderer&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    program_ = std::move(rhs.program_);
    return *this;
}

Shape2D Shape2DRenderer::create_shape(const Buffer& buffer, const Xyz::Vector4F& color) const
{
    Tungsten::use_program(program_->program);
    auto vertex_array = Tungsten::VertexArrayBuilder<Xyz::Vector2F>()
        .add_float(program_->position_attr, 2)
        .build();
    set_buffers(vertex_array, buffer);
    return {std::move(vertex_array), color};
}

void Shape2DRenderer::set_model_view_matrix(const Xyz::Matrix3F& matrix)
{
    model_view_matrix_ = matrix;
}

void Shape2DRenderer::set_projection_matrix(const Xyz::Matrix3F& matrix)
{
    projection_matrix_ = matrix;
}

void Shape2DRenderer::draw(const Shape2D& shape)
{
    shape.vertex_array().bind();
    Tungsten::use_program(program_->program);
    program_->color.set(shape.color());
    program_->model_view_matrix.set(model_view_matrix_);
    program_->projection_matrix.set(projection_matrix_);
    program_->z.set(0);
    Tungsten::draw_triangle_elements_16(0, 6);
}
