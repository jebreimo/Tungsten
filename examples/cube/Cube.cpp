//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>
#include <Yconvert/Convert.hpp>
#include <Ystring/Ystring.hpp>

#include "MeshItem.hpp"
#include "Resources.hpp"

Tungsten::VertexArrayObject
make_cube_vao(const Tungsten::SmoothShader& program,
              bool wireframe)
{
    Tungsten::VertexArrayData<Tungsten::PositionNormalTexture> cube;
    Tungsten::VertexArrayDataBuilder builder(cube);
    Tungsten::add_cube_pnt(builder);
    if (!wireframe)
    {
        write_pnt(std::cout, cube);
    }
    else
    {
        cube.indices = Tungsten::triangles_to_line_segments(cube.indices);
        const auto& vertices = cube.vertices;
        Tungsten::write_pnt(std::cout, std::span(vertices), true);
        Tungsten::write_line_segments(std::cout, cube.indices);
    }

    int32_t extra_stride = sizeof(Xyz::Vector2F);
    // if (!dynamic_cast<const Tungsten::TexturedSmoothShader*>(&program))
    //     extra_stride = sizeof(Xyz::Vector2F);
    auto vao = program.create_vao(extra_stride);

    vao.set_data(cube.vertices, cube.indices);

    return vao;
}

Tungsten::SmoothShader& get_shader(Tungsten::BuiltinShader shader_type)
{
    return dynamic_cast<Tungsten::SmoothShader&>(
        Tungsten::ShaderManager::instance().program(shader_type));
}

Tungsten::Camera make_camera(const Tungsten::Viewport viewport)
{
    const auto fov_y = Tungsten::calculate_fov_y(
        Xyz::to_radians(50.f), Xyz::to_radians(50.f), viewport.aspect_ratio());
    return Tungsten::CameraBuilder()
        .viewport(viewport)
        .look_at({-2, -sqrt(17.f), 2}, {0, 0, 0}, {0, 0, 1})
        .perspective(fov_y, 1.5f, 10)
        .build();
}

Tungsten::TextureHandle make_texture(const Yimage::Image& image)
{
    auto handle = Tungsten::generate_texture();
    bind_texture(Tungsten::TextureTarget::TEXTURE_2D, handle);
    set_min_filter(Tungsten::TextureTarget::TEXTURE_2D, Tungsten::TextureMinFilter::LINEAR);
    set_mag_filter(Tungsten::TextureTarget::TEXTURE_2D, Tungsten::TextureMagFilter::LINEAR);
    set_wrap(Tungsten::TextureTarget::TEXTURE_2D, Tungsten::TextureWrapMode::CLAMP_TO_EDGE);

    set_texture_image_2d(
        Tungsten::TextureTarget2D::TEXTURE_2D,
        0,
        {int32_t(image.width()), int32_t(image.height())},
        Tungsten::get_ogl_pixel_type(image.pixel_type()),
        image.data());

    return handle;
}

Yimage::Image get_image()
{
    return Tungsten::read_image(NUMBERS_PNG, std::size(NUMBERS_PNG) - 1);
}

class Cube : public Tungsten::EventLoop
{
public:
    Cube(Tungsten::SdlApplication& app,
                 Tungsten::BuiltinShader shader_type,
                 bool wireframe)
        : EventLoop(app),
          texture_(make_texture(get_image())),
          program(get_shader(shader_type)),
          item(make_cube_vao(program, wireframe), {}, wireframe),
          camera{make_camera(app.viewport())}
    {
        item.set_texture(texture_);
        item.set_material(Tungsten::get_default_material(Tungsten::DefaultMaterial::RED_PLASTIC));
        std::cout << Tungsten::get_device_info() << '\n';
        Tungsten::set_swap_interval(app, Tungsten::SwapInterval::VSYNC);
    }

    bool on_event(const SDL_Event& event) override
    {
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            camera = make_camera(application().viewport());
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
            return Xyz::affine::rotate_z<float>(-angle);
        return Xyz::affine::rotate_y(angle);
    }

    Tungsten::TextureHandle texture_;
    Tungsten::SmoothShader& program;
    MeshItem item;
    Tungsten::Camera camera;
    uint64_t start_ticks = SDL_GetTicks();
};

argos::ParsedArguments parse_arguments(int argc, char* argv[])
{
    using namespace argos;
    ArgumentParser parser;
    parser
        .add(Opt("-s", "--shader")
            .argument("name")
            .help("Name of the shader program to use. Available shaders are"
                " listed below, default is TEXTURED_BLINN_PHONG."))
        .add(Opt("-w", "--wireframe")
            .help("Show the cube in wireframe mode."))
        .text(TextId::FINAL_TEXT,
              "Available shaders:\n"
              "  TEXTURED_BLINN_PHONG\n"
              "  TEXTURED_PHONG\n"
              "  TEXTURED_GOURAUD\n"
              "  BLINN_PHONG\n"
              "  PHONG\n"
              "  GOURAUD");
    Tungsten::SdlApplication::add_command_line_options(parser);
    return parser.parse(argc, argv);
}

Tungsten::BuiltinShader get_shader_type(const argos::ParsedArguments& args)
{
    const auto name = ystring::to_upper(args.value("--shader").as_string("SMOOTH"));
    return Tungsten::to_builtin_shader(name);
}

int main(int argc, char** argv)
{
    try
    {
        const auto args = parse_arguments(argc, argv);
        Tungsten::SdlApplication app("TexturedCube");
        app.read_command_line_options(args);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<Cube>(get_shader_type(args),
                              args.value("--wireframe").as_bool(false));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
