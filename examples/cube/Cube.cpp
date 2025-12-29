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

#include "MeshItem.hpp"
#include "Tungsten/MeshShapes.hpp"
#include "Tungsten/ShaderManager.hpp"

Tungsten::VertexArrayObject make_cube_vao(const Tungsten::SmoothMeshShader& program)
{
    Tungsten::VertexArrayData<Tungsten::PositionNormal> cube;
    Tungsten::VertexArrayDataBuilder builder(cube);
    add_cube_pn(builder);
    write(std::cout, cube);
    auto vao = program.create_vao();
    vao.set_data(cube.vertices, cube.indices);
    return vao;
}

Tungsten::SmoothMeshShader& get_phong_shader()
{
    return dynamic_cast<Tungsten::SmoothMeshShader&>(
        Tungsten::ShaderManager::instance().program(Tungsten::BuiltinShaders::PHONG));
}

Tungsten::Camera make_camera(float aspect_ratio)
{
    const auto fov_y = Tungsten::calculate_fov_y(
        Xyz::to_radians(50.f), Xyz::to_radians(50.f), aspect_ratio);
    return Tungsten::Camera{
        Xyz::make_look_at_matrix<float>({-2, -sqrt(17.f), 2}, {0, 0, 0}, {0, 0, 1}),
        Xyz::make_perspective_matrix<float>(fov_y, aspect_ratio, 1.5f, 10)
    };
}

class Cube : public Tungsten::EventLoop
{
public:
    explicit Cube(Tungsten::SdlApplication& app)
        : EventLoop(app),
          program(get_phong_shader()),
          item(make_cube_vao(program), {}),
          camera{make_camera(app.aspect_ratio())}
    {
        std::cout << Tungsten::get_device_info() << '\n';
    }

    bool on_event(const SDL_Event& event) override
    {
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            const auto aspect_ratio = float(event.window.data1) / float(event.window.data2);
            camera = make_camera(aspect_ratio);
            return true;
        }
        return false;
    }

    void on_update() override
    {
        item.set_model_matrix(get_rotation(SDL_GetTicks() - start_ticks));
    }

    void on_draw() override
    {
        Tungsten::set_depth_test_enabled(true);
        Tungsten::set_face_culling_enabled(true);
        Tungsten::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
        Tungsten::clear(Tungsten::ClearBits::COLOR_DEPTH);
        auto [w, h] = application().window_size();
        Tungsten::set_viewport(0, 0, w, h);

        program.use();
        program.set_projection_matrix(camera.projection);

        item.draw(camera, program);

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
    MeshItem item;
    Tungsten::Camera camera;
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
