//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-06.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// Normal mapping in the builtin BlinnPhong family, as a side-by-side: two
// cubes of the same colour material, the left one with a normal map and the
// right one without. The only difference between their materials is the
// third texture and the strength passed to
// make_blinn_phong_material_params.
//
// The family needs no TANGENT attribute — the shader builds a tangent basis
// from screen-space derivatives — so both cubes share one mesh with the
// ordinary position / normal / texcoord layout.
//
// The pair rotates under a fixed directional light. That movement is the
// point: a normal map is barely visible in a still frame and unmistakable
// once the highlight sweeps across the studs.

#include <cmath>
#include <iostream>
#include <vector>
#include <Tungsten/Tungsten.hpp>
#include <Tungsten/Gl/IOglWrapper.hpp>
#include <Xyz/Mesh/BuildMesh.hpp>

namespace
{
    using namespace Tungsten;

    constexpr int NORMAL_MAP_SIZE = 512;
    // Studs per cube face, and their radius as a fraction of the half-cell.
    // Below 1 the studs stay clear of the cell borders, which keeps the map's
    // edges flat and its seams invisible.
    constexpr int STUDS_PER_EDGE = 4;
    constexpr float STUD_RADIUS = 0.7f;

    constexpr float HALF_GAP = 1.6f;

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

        constexpr Xyz::OrientedCuboid<float> cuboid{
            .placement = {.origin = {-1, -1, -1}},
            .size = {2, 2, 2}
        };
        // Every face gets the whole unit square, so the stud pattern repeats
        // once per face. The rectangle is not optional: without it build_mesh
        // gives every vertex the texture coordinate (0, 0), which leaves the
        // surface with no texture gradient at all — and a tangent basis built
        // from derivatives needs that gradient, so the normal map would have
        // no effect whatsoever.
        auto unit_square = [](int) { return Xyz::RectangleF{{0, 0}, {1, 1}}; };
        Xyz::build_mesh(builder, cuboid, std::function(unit_square));
        return {std::move(indexes), std::move(vertexes)};
    }

    /**
     * A grid of hemispherical studs as an RGB tangent-space normal map: the
     * unit normal mapped from [-1, 1] to [0, 1]. Flat ground comes out as
     * (128, 128, 255), the colour that leaves the surface normal untouched.
     */
    std::vector<uint8_t> make_stud_normal_map()
    {
        std::vector<uint8_t> pixels(size_t(NORMAL_MAP_SIZE) * NORMAL_MAP_SIZE * 3);
        const float cell = float(NORMAL_MAP_SIZE) / float(STUDS_PER_EDGE);

        for (int y = 0; y < NORMAL_MAP_SIZE; ++y)
        {
            for (int x = 0; x < NORMAL_MAP_SIZE; ++x)
            {
                // Where this texel sits in its own stud's cell, in [-1, 1].
                const float u = 2 * std::fmod(float(x), cell) / cell - 1;
                const float v = 2 * std::fmod(float(y), cell) / cell - 1;

                const float a = u / STUD_RADIUS;
                const float b = v / STUD_RADIUS;
                const float dist2 = a * a + b * b;

                // On the dome the normal is (a, b, sqrt(1 - a^2 - b^2)), which
                // is a unit vector by construction — no normalize needed.
                const auto normal = dist2 < 1.0f
                                        ? Xyz::Vector3F{a, b, std::sqrt(1 - dist2)}
                                        : Xyz::Vector3F{0, 0, 1};

                const auto texel = 3 * (size_t(y) * NORMAL_MAP_SIZE + x);
                for (int c = 0; c < 3; ++c)
                {
                    pixels[texel + size_t(c)] =
                        uint8_t(std::lround(127.5f * (normal[c] + 1)));
                }
            }
        }
        return pixels;
    }

    class BumpCube : public EventLoop
    {
    public:
        explicit BumpCube(SdlApplication& app)
            : EventLoop(app),
              renderer_(resources_)
        {
            std::cout << get_device_info() << '\n';
            register_builtin_shader_families(resources_);
            const auto shader = resources_.register_shader_variant(
                {.family = BLINN_PHONG_FAMILY});

            const auto mesh = make_cube_mesh();

            // Both cubes draw through one pipeline: they differ only in their
            // material parameters and textures, which is material state, not
            // pipeline state.
            PipelineDescriptor descriptor;
            descriptor.shader = shader;
            descriptor.layout = cube_layout();
            descriptor.raster.cull_enabled = true;
            const auto pipeline = resources_.register_pipeline(descriptor);

            // Same colour material for both, so every visible difference comes
            // from the normal map.
            const auto bumped = make_material(pipeline, 1.0f, true);
            const auto plain = make_material(pipeline, 0.0f, false);

            hub_ = scene_.add_node();
            add_cube(mesh, bumped, -HALF_GAP);
            add_cube(mesh, plain, HALF_GAP);

            camera_ = scene_.add_node();
            camera_.set_local_transform({
                .translation = {0, 2, 8},
                .rotation = look_at_rotation({0, 2, 8}, {0, 0, 0})
            });
            camera_.add(CameraComponent{
                .near_plane = 0.5f,
                .far_plane = 50.0f
            });

            auto light_node = scene_.add_node();
            // A directional light shines along its node's -z axis; tilt the
            // node so the light comes in from the upper right.
            light_node.set_local_transform({
                .rotation = euler_rotation(-0.6f, 0.4f, 0.0f)
            });
            light_node.add(LightComponent{
                .type = LightType::DIRECTIONAL,
                .color = {1.0f, 0.97f, 0.9f},
                .intensity = 1.0f
            });

            set_swap_interval(app, SwapInterval::VSYNC);
        }

        void on_update() override
        {
            const auto t = float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            // Rotating the hub orbits both cubes and turns each about its own
            // axis, which is what sweeps the highlight over the studs.
            hub_.set_local_transform({.rotation = euler_rotation(0.5f * t, 0.25f * t, 0)});
        }

        void on_draw() override
        {
            const auto viewport = application().viewport();
            const RenderPassDescriptor pass{
                .viewport = viewport,
                .color = {.clear_color = {0.14f, 0.15f, 0.18f, 1.0f}}
            };
            camera_.get<CameraComponent>().aspect = viewport.aspect_ratio();

            resources_.begin_frame(frame_);

            scene_.resolve_transforms();
            builder_.build(scene_, camera_.id(), snapshots_.back());
            snapshots_.back().time =
                float(SDL_GetTicks() - start_ticks_) / 1000.0f;
            snapshots_.back().ambient_light =
                srgb_to_linear(Xyz::Vector3F{0.30f, 0.30f, 0.33f});
            snapshots_.swap();

            renderer_.render(snapshots_.front(), pass);

            set_ogl_tracing_enabled(false);

            resources_.collect_garbage(frame_);
            ++frame_;
            redraw();
        }

    private:
        /**
         * @param strength what goes in u_ambient.w: zero means the material has
         *      no normal map and the shader skips the sampler entirely.
         * @param with_map whether to attach the map. The renderer binds
         *      textures to consecutive units, so the normal map can only reach
         *      unit 2 behind two others — hence the white filler.
         */
        MaterialRef make_material(PipelineRef pipeline, float strength,
                                  bool with_map)
        {
            Material material;
            material.pipeline = pipeline;
            material.parameter_data = make_blinn_phong_material_params(
                StandardColorMaterial::COPPER, 1.0f, strength);
            // The renderer binds textures to consecutive units, so the normal
            // map can only reach unit 2 behind two others. Null refs get the
            // renderer's white fallback, which leaves the material's own
            // colours unmodulated.
            if (with_map)
                material.textures = {{}, {}, normal_map()};
            return resources_.create_material(std::move(material));
        }

        TextureRef normal_map()
        {
            if (!normal_map_)
            {
                const auto pixels = make_stud_normal_map();
                // DATA, not COLOR: these texels are a tangent-space vector
                // field. Decoding them as sRGB would not dim the bumps, it
                // would bend every normal the wrong way.
                normal_map_ = resources_.create_texture({
                    .size = {NORMAL_MAP_SIZE, NORMAL_MAP_SIZE},
                    .format = RGB_TEXTURE,
                    .content = TextureContent::DATA,
                    .pixels = pixels.data(),
                    .sampler = resources_.register_sampler({
                        .mip_filter = SamplerMipFilter::NONE,
                        .address_mode_u = SamplerAddressMode::CLAMP_TO_EDGE,
                        .address_mode_v = SamplerAddressMode::CLAMP_TO_EDGE
                    })
                });
            }
            return normal_map_;
        }

        // Named by both the mesh and the pipeline: a pipeline pins how the
        // attributes are packed, so the two have to agree.
        VertexLayoutRef cube_layout()
        {
            return resources_.register_layout(
                VertexLayoutBuilder()
                    .add_attribute(AttributeSemantic::POSITION)
                    .add_attribute(AttributeSemantic::NORMAL)
                    .add_attribute(AttributeSemantic::TEX_COORD_0)
                    .build());
        }

        MeshRef make_cube_mesh()
        {
            auto [indexes, vertexes] = ::make_cube_mesh();

            constexpr size_t STRIDE = 8; // 3 coords + 3 normals + 2 tex coords

            const auto layout = cube_layout();
            vbo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, STRIDE * sizeof(float),
                uint32_t(vertexes.size() / STRIDE));
            ebo_arena_ = resources_.create_arena(
                BufferUsage::STATIC_DRAW, sizeof(uint16_t),
                uint32_t(indexes.size()));

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

            Mesh mesh;
            mesh.streams = {vertices};
            mesh.layout = layout;
            mesh.ebo = indices;
            mesh.index_type = ElementIndexType::UINT16;
            return resources_.create_mesh(std::move(mesh));
        }

        void add_cube(MeshRef mesh, MaterialRef material, float x)
        {
            auto node = hub_.add_child();
            node.set_local_transform({.translation = {x, 0, 0}});
            node.add(RenderableComponent{
                .mesh = mesh,
                .material = material,
                .local_bounds = {{-1, -1, -1}, {1, 1, 1}}
            });
        }

        ResourceManager resources_;
        Scene scene_;
        DoubleBuffer<RenderSnapshot> snapshots_;
        SnapshotBuilder builder_{resources_};
        Renderer renderer_;
        BufferArenaRef vbo_arena_;
        BufferArenaRef ebo_arena_;
        TextureRef normal_map_;
        NodeHandle hub_;
        NodeHandle camera_;
        uint64_t frame_ = 0;
        uint64_t start_ticks_ = SDL_GetTicks();
    };
}

int main(int argc, char** argv)
{
    try
    {
        Tungsten::SdlApplication app("BumpCube");
        app.parse_command_line_options(argc, argv);
        Tungsten::set_ogl_tracing_enabled(true);
        app.run<BumpCube>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
