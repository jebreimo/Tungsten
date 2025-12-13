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

#include "Tungsten/PositionNormalShapes.hpp"
#include "Tungsten/ShaderManager.hpp"

Tungsten::VertexArrayData<Tungsten::PositionNormal> make_cube_array()
{
    Tungsten::VertexArrayData<Tungsten::PositionNormal> buffer;
    Tungsten::VertexArrayDataBuilder builder(buffer);
    add_cube(builder);
    return buffer;
}

class Cube : public Tungsten::EventLoop
{
public:
    explicit Cube(Tungsten::SdlApplication& app)
        : EventLoop(app),
          program(dynamic_cast<Tungsten::SmoothMeshShader&>(
              Tungsten::ShaderManager::instance().get_program(Tungsten::BuiltinShaders::PHONG)))
    {
        std::cout << Tungsten::get_device_info() << '\n';

        auto cube = make_cube_array();
        std::cout << cube << '\n';
        vao = program.create_vao();
        vao.set_data(cube.vertexes, cube.indexes);
    }

    void on_draw() override
    {
        Tungsten::set_depth_test_enabled(true);
        Tungsten::set_face_culling_enabled(true);
        Tungsten::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
        Tungsten::clear(Tungsten::ClearBufferMask::COLOR_DEPTH);
        auto [w, h] = application().window_size();
        Tungsten::set_viewport(0, 0, w, h);

        program.use();
        program.set_model_view_matrix(
            Xyz::make_look_at_matrix<float>({-2, -sqrt(17.f), 2}, {0, 0, 0}, {0, 0, 1})
            * get_rotation(SDL_GetTicks() - start_ticks)
        );
        program.set_projection_matrix(
            Xyz::make_perspective_matrix<float>(Xyz::to_radians(50.f), float(w) / float(h), 1.5f,
                                                10)
        );
        vao.bind();
        Tungsten::draw_triangle_elements_16(0, vao.element_count);

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

    Tungsten::SmoothMeshShader& program;
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
