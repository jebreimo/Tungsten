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

#include "Resources.hpp"

namespace
{
    std::pair<std::vector<uint16_t>, std::vector<float>> make_cube_mesh()
    {
        std::vector<uint16_t> indexes;
        std::vector<float> vertexes;

        using Builder2F = Xyz::MeshAttributeBuilder<Xyz::Vector2F, std::vector<float>>;
        using Builder3F = Xyz::MeshAttributeBuilder<Xyz::Vector3F, std::vector<float>>;

        Xyz::MeshBuilder builder{
            .indexes = Xyz::MeshIndexBuilder(indexes),
            .coords = Builder3F(vertexes, 8),
            .normals = std::optional(Builder3F(vertexes, 8, 3)),
            .tex_coords = std::optional(Builder2F(vertexes, 8, 6))
        };

        auto get_tex_rect = [](int i)
        {
            return Xyz::RectangleF{{float(i) / 6, 0}, {1.f / 6, 1}};
        };

        Xyz::OrientedCuboid<float> cuboid{{{-1, -1, -1}, {0, 0, 0}}, {2, 2, 2}};
        Xyz::build_mesh(builder, cuboid, std::function(get_tex_rect));
        return {std::move(indexes), std::move(vertexes)};
    }

    Tungsten::SmoothShader& get_shader(Tungsten::BuiltinShader shader_type)
    {
        return dynamic_cast<Tungsten::SmoothShader&>(
            Tungsten::ShaderManager::instance().program(shader_type));
    }

    Tungsten::Camera make_camera(const Tungsten::Viewport viewport)
    {
        const auto fov_y = Tungsten::calc_fov_y(
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
        bind_texture(Tungsten::TextureTarget::TEXTURE_2D, handle.id());
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
        Cube(Tungsten::SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            Tungsten::register_builtin_shader_families(resources_);
            const auto shader = resources_.register_shader_variant(
                {.family = Tungsten::BLINN_PHONG_FAMILY});

            const auto gold = make_material(
                shader, false,
                Tungsten::make_blinn_phong_material_params(
                    Tungsten::StandardColorMaterial::GOLD,
                    1.0f));

            const auto mesh = make_cube_mesh();

            hub_ = &scene_.add(std::make_unique<Tungsten::Node>());
            add_renderable(*hub_, mesh, gold);

            auto& camera_node = scene_.add(std::make_unique<Tungsten::Node>());
            Tungsten::Transform camera_transform;
            camera_transform.translation = {0, 0, 8};
            camera_node.set_local_transform(camera_transform);
            auto camera = std::make_unique<Tungsten::CameraComponent>();
            camera->near_plane = 0.5f;
            camera->far_plane = 50.0f;
            camera->aspect = app.viewport().aspect_ratio();
            camera_ = &camera_node.add_component(std::move(camera));

            auto& light_node = scene_.add(std::make_unique<Tungsten::Node>());
            Tungsten::Transform light_transform;
            // A directional light shines along its node's -z axis; tilt the
            // node so the light comes in from the upper right.
            light_transform.rotation = Tungsten::euler_rotation(-0.6f, 0.4f, 0.0f);
            light_node.set_local_transform(light_transform);
            auto light = std::make_unique<Tungsten::LightComponent>();
            light->type = Tungsten::LightType::DIRECTIONAL;
            light->color = {1.0f, 0.97f, 0.9f};
            light->intensity = 1.0f;
            light_node.add_component(std::move(light));

            std::cout << Tungsten::get_device_info() << '\n';
            Tungsten::set_swap_interval(app, Tungsten::SwapInterval::VSYNC);
        }

        bool on_event(const SDL_Event& event) override
        {
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                return true;
            }
            return false;
        }

        void on_update() override
        {
            const auto t = float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            Tungsten::Transform hub;
            hub.rotation = Tungsten::euler_rotation(0.4f * t, 0.2f * t, 0.0f);
            hub_->set_local_transform(hub);
        }

        void on_draw() override
        {
            Tungsten::set_depth_test_enabled(true);
            Tungsten::set_face_culling_enabled(true);
            Tungsten::set_clear_color({0.2f, 0.3f, 0.3f, 1.0f});
            Tungsten::clear(Tungsten::ClearBits::COLOR_DEPTH);
            auto [w, h] = application().window_size();
            Tungsten::set_viewport(0, 0, w, h);

            resources_.begin_frame(frame_);

            Tungsten::TransformUpdater::resolve(scene_);
            auto& snapshots = scene_.snapshots();
            builder_.build(scene_, *camera_, snapshots.back());
            snapshots.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots.back().ambient_light = {0.35f, 0.35f, 0.38f};
            snapshots.swap();

            renderer_.render(snapshots.front());

            Tungsten::set_ogl_tracing_enabled(false);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        Tungsten::MaterialRef make_material(Tungsten::ShaderProgramRef shader,
                                            bool transparent,
                                            std::vector<std::byte> params)
        {
            Tungsten::Material material;
            material.shader = shader;
            material.parameter_data = std::move(params);
            material.transparent = transparent;
            material.textures = {make_texture(get_image())};
            return resources_.create_material(std::move(material));
        }

        Tungsten::MeshRef make_cube_mesh()
        {
            auto [indexes, vertexes] = ::make_cube_mesh();

            constexpr size_t STRIDE = 8; // 3 coords + 3 normals + 2 tex coords

            const auto layout = resources_.register_layout(
                Tungsten::builtin_pnt_layout());
            vbo_arena_ = resources_.create_arena(
                Tungsten::BufferUsage::STATIC_DRAW, STRIDE * sizeof(float),
                uint32_t(vertexes.size() / STRIDE));
            ebo_arena_ = resources_.create_arena(
                Tungsten::BufferUsage::STATIC_DRAW, sizeof(uint16_t), uint32_t(indexes.size()));

            const auto vertices = resources_.allocate(
                vbo_arena_, uint32_t(vertexes.size() / STRIDE));
            const auto indices = resources_.allocate(
                ebo_arena_, uint32_t(indexes.size()));

            // Rebase to absolute indices (§3): the portable path has no
            // baseVertex draw argument.
            for (auto& index : indexes)
                index = uint16_t(index + vertices.offset);

            resources_.upload(vertices, vertexes.data(),
                              vertexes.size() * sizeof(float));
            resources_.upload(indices, indexes.data(),
                              indexes.size() * sizeof(uint16_t));

            Tungsten::Mesh mesh;
            const Tungsten::BufferArenaRef vbos[] = {vbo_arena_};
            mesh.vao = resources_.get_vao(vbos, ebo_arena_, layout);
            mesh.streams = {vertices};
            mesh.layout = layout;
            mesh.ebo = indices;
            mesh.index_type = Tungsten::ElementIndexType::UINT16;
            mesh.primitive = Tungsten::TopologyType::TRIANGLES;
            return resources_.create_mesh(std::move(mesh));
        }

        Tungsten::TextureRef make_texture(const Yimage::Image& image)
        {
            auto handle = ::make_texture(image);
            Tungsten::Texture texture;
            texture.gl_handle = std::move(handle);
            texture.width = image.width();
            texture.height = image.height();
            texture.format = Tungsten::TextureFormat::RGBA;
            return resources_.create_texture(std::move(texture));
        }

        void add_renderable(Tungsten::Node& node, Tungsten::MeshRef mesh, Tungsten::MaterialRef material)
        {
            auto renderable = std::make_unique<Tungsten::RenderableComponent>();
            renderable->mesh = mesh;
            renderable->material = material;
            renderable->local_bounds = {{-1, -1, -1}, {1, 1, 1}};
            node.add_component(std::move(renderable));
        }

        static Xyz::Matrix4F get_rotation(uint64_t ticks)
        {
            double i;
            const auto fraction = std::modf(double(ticks) / 5000, &i);
            const auto angle = float(fraction * 2 * Xyz::Constants<double>::PI);
            if ((ticks / 10000) % 2 == 0)
                return Xyz::affine::rotate_z<float>(-angle);
            return Xyz::affine::rotate_y(angle);
        }

        Tungsten::ResourceManager resources_;
        Tungsten::Scene scene_;
        Tungsten::SnapshotBuilder builder_{resources_};
        Tungsten::Renderer renderer_;
        Tungsten::BufferArenaRef vbo_arena_;
        Tungsten::BufferArenaRef ebo_arena_;
        Tungsten::Node* hub_ = nullptr;
        Tungsten::CameraComponent* camera_ = nullptr;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };

    argos::ParsedArguments parse_arguments(int argc, char* argv[])
    {
        using namespace argos;
        ArgumentParser parser;
        parser
            .add(Opt("-s", "--shader")
                .argument("name")
                .help("Name of the shader program to use. Available shaders are"
                    " listed below, default is SMOOTH."))
            .add(Opt("-w", "--wireframe")
                .help("Show the cube in wireframe mode."))
            .text(TextId::FINAL_TEXT,
                  "Available shaders:\n"
                  "  SMOOTH\n"
                  "  BLINN_PHONG\n");
        Tungsten::SdlApplication::add_command_line_options(parser);
        return parser.parse(argc, argv);
    }

    Tungsten::BuiltinShader get_shader_type(const argos::ParsedArguments& args)
    {
        const auto name = ystring::to_upper(args.value("--shader").as_string("SMOOTH"));
        return Tungsten::to_builtin_shader(name);
    }
}

int main(int argc, char** argv)
{
    try
    {
        const auto args = parse_arguments(argc, argv);
        Tungsten::SdlApplication app("TexturedCube");
        app.read_command_line_options(args);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<Cube>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
