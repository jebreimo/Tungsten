//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Shape2D.hpp"

#include "Tungsten/Tungsten.hpp"
#include "Resources.hpp"

class Shape2DRenderer::Basic2DProgram
{
public:
    Basic2DProgram()
    {
        const Tungsten::ShaderPreprocessor pp;
        program = Tungsten::ShaderProgramBuilder()
            .add_shader(Tungsten::ShaderType::VERTEX,
                        pp.preprocess(std::string_view(BASIC2D_VERTEX)))
            .add_shader(Tungsten::ShaderType::FRAGMENT,
                        pp.preprocess(std::string_view(BASIC2D_FRAGMENT)))
            .build();

        position_attr = Tungsten::get_vertex_attribute(program.id(), "a_position");
        model_view_matrix = Tungsten::get_uniform<Xyz::Matrix3F>(program.id(), "u_model_view_matrix");
        projection_matrix = Tungsten::get_uniform<Xyz::Matrix3F>(program.id(), "u_projection_matrix");
        color = Tungsten::get_uniform<Xyz::Vector4F>(program.id(), "u_color");
        z = Tungsten::get_uniform<float>(program.id(), "u_z");
        point_size = Tungsten::get_uniform<float>(program.id(), "u_point_size");
    }

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix3F> model_view_matrix;
    Tungsten::Uniform<Xyz::Matrix3F> projection_matrix;
    Tungsten::Uniform<Xyz::Vector4F> color;
    Tungsten::Uniform<float> z;
    Tungsten::Uniform<float> point_size;

    uint32_t position_attr;
};

Shape2D::Shape2D(Tungsten::VertexArrayObject vertex_array,
                 Tungsten::BufferHandle vertex_buffer,
                 Tungsten::BufferHandle element_buffer,
                 uint32_t element_count,
                 const Xyz::Vector4F& color)
    : vertex_array_(std::move(vertex_array)),
      vertex_buffer_(std::move(vertex_buffer)),
      element_buffer_(std::move(element_buffer)),
      element_count_(element_count),
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

const Tungsten::VertexArrayObject& Shape2D::vertex_array() const
{
    return vertex_array_;
}

void Shape2D::bind() const
{
    vertex_array_.bind();
    Tungsten::bind_buffer(Tungsten::BufferTarget::ELEMENT_ARRAY, element_buffer_.id());
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
    Tungsten::use_program(program_->program.id());
    auto vertex_buffer = Tungsten::generate_buffer();
    auto vertex_array = Tungsten::VertexArrayObjectBuilder()
        .bind_buffer(vertex_buffer.id())
        .add_float(program_->position_attr, 2)
        .build();
    Tungsten::bind_buffer(Tungsten::BufferTarget::ARRAY, vertex_buffer.id());
    Tungsten::set_buffer_data(Tungsten::BufferTarget::ARRAY, std::span(buffer.vertices),
                            Tungsten::BufferUsage::STATIC_DRAW);
    auto element_buffer = Tungsten::generate_buffer();
    Tungsten::bind_buffer(Tungsten::BufferTarget::ELEMENT_ARRAY, element_buffer.id());
    Tungsten::set_buffer_data(Tungsten::BufferTarget::ELEMENT_ARRAY, std::span(buffer.indices),
                            Tungsten::BufferUsage::STATIC_DRAW);
    return {
        std::move(vertex_array),
        std::move(vertex_buffer),
        std::move(element_buffer),
        uint32_t(buffer.indices.size()),
        color
    };
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
    shape.bind();
    Tungsten::use_program(program_->program.id());
    program_->color.set(shape.color());
    program_->model_view_matrix.set(model_view_matrix_);
    program_->projection_matrix.set(projection_matrix_);
    program_->z.set(0);
    Tungsten::draw_triangle_elements_16(0, 6);
}
