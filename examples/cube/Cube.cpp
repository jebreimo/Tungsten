//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include <Yconvert/Convert.hpp>

#include "Tungsten/PhongShaderProgram.hpp"

struct PosNormal
{
    Xyz::Vector3F pos;
    Xyz::Vector3F normal;
};

struct PosNormalTex
{
    Xyz::Vector3F pos;
    Xyz::Vector3F normal;
    Xyz::Vector2F tex;
};

std::ostream& operator<<(std::ostream& os, const PosNormal& p)
{
    return os << p.pos << " " << p.normal;
}

std::ostream& operator<<(std::ostream& os, const Tungsten::VertexArrayData<PosNormal>& buffer)
{
    os << "Vertexes:\n";
    for (const auto& v : buffer.vertexes)
        os << v << '\n';
    os << "Indexes:";
    for (size_t i = 0; i < buffer.indexes.size(); i += 3)
    {
        os << '\n' << buffer.indexes[i] << ' '
            << buffer.indexes[i + 1] << ' '
            << buffer.indexes[i + 2];
    }
    return os;
}

void add_rect(Tungsten::VertexArrayDataBuilder<PosNormal> builder,
              const Xyz::Rectangle3F& rect)
{
    const auto normal = rect.normal_vector();
    for (int i = 0; i < 4; ++i)
        builder.add_vertex({rect[i], normal});

    builder.add_indexes(0, 1, 2);
    builder.add_indexes(0, 2, 3);
}

Tungsten::VertexArrayData<PosNormal> make_cube_array()
{
    constexpr auto PI = Xyz::Constants<float>::PI;
    using O3F = Xyz::Orientation3F;
    Tungsten::VertexArrayData<PosNormal> buffer;
    using Builder = Tungsten::VertexArrayDataBuilder<PosNormal>;
    add_rect(Builder(buffer, 0), {{{-1, -1, 1}, O3F{0, 0, 0}}, {2, 2}});
    add_rect(Builder(buffer, 4), {{{-1, -1, 1}, O3F{PI / 2, 0, -PI / 2}}, {2, 2}});
    add_rect(Builder(buffer, 8), {{{-1, -1, -1}, O3F{0, 0, PI / 2}}, {2, 2}});
    add_rect(Builder(buffer, 12), {{{-1, 1, -1}, O3F{0, 0, PI}}, {2, 2}});
    add_rect(Builder(buffer, 16), {{{1, -1, -1}, O3F{PI / 2, 0, PI / 2}}, {2, 2}});
    add_rect(Builder(buffer, 20), {{{-1, 1, 1}, O3F{0, 0, -PI / 2}}, {2, 2}});
    return buffer;
}

class Cube : public Tungsten::EventLoop
{
public:
    explicit Cube(Tungsten::SdlApplication& app)
        : EventLoop(app)
    {
        std::cout << Tungsten::get_device_info() << '\n';

        auto cube = make_cube_array();
        std::cout << cube;
        vao = program.create_vao();
        vao.set_data<PosNormal>(cube.vertexes, cube.indexes);
    }

    bool on_event(const SDL_Event& event) override
    {
        return false;
    }

    void on_update() override
    {
    }

    void on_draw() override
    {
        Tungsten::set_depth_test_enabled(true);
        Tungsten::set_face_culling_enabled(true);
        Tungsten::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
        Tungsten::clear(Tungsten::ClearBufferMask::COLOR_DEPTH);
        auto [w, h] = application().window_size();

        program.use();
        program.mv_matrix.set(
            Xyz::make_look_at_matrix<float>({-2, -sqrt(17.f), 2}, {0, 0, 0}, {0, 0, 1})
            * get_rotation(SDL_GetTicks() - start_ticks)
        );
        program.proj_matrix.set(
            Xyz::make_perspective_matrix<float>(Xyz::to_radians(50.f), float(w) / float(h), 1.5f,
                                                10)
        );
        vao.bind();
        Tungsten::draw_triangle_elements_16(0, vao.index_count);

        Tungsten::set_ogl_tracing_enabled(false);
        redraw();
    }

private:
    static Xyz::Matrix4F get_rotation(uint64_t ticks)
    {
        double i;
        const auto fraction = std::modf(double(ticks) / 5000, &i);
        const auto angle = float(fraction * 2 * Xyz::Constants<double>::PI);
        if ((ticks / 10000) % 2 == 0)
            return Xyz::affine::rotate_z<float>(angle);
        return Xyz::affine::rotate_y(angle);
    }

    Tungsten::PhongShaderProgram program;
    Tungsten::VertexArrayObject vao;
    uint64_t start_ticks = SDL_GetTicks();
};

int main(int argc, char** argv)
{
    try
    {
        Tungsten::SdlApplication app("Cube");
        app.parse_command_line_options(argc, argv);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<Cube>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
