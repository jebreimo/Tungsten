//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BumpCubeModel.hpp"

#include <cmath>
#include <vector>
#include <Tungsten/Tungsten.hpp>
#include <Xyz/Mesh/BuildMesh.hpp>

namespace
{
    using namespace Tungsten;

    // 3 coords + 3 normals + 2 tex coords
    constexpr size_t STRIDE = 8;

    constexpr int NORMAL_MAP_SIZE = 512;
    // Studs per cube face, and their radius as a fraction of the half-cell.
    // Below 1 the studs stay clear of the cell borders, which keeps the map's
    // edges flat and its seams invisible.
    constexpr int STUDS_PER_EDGE = 4;
    constexpr float STUD_RADIUS = 0.7f;

    constexpr float HALF_GAP = 1.6f;

    std::pair<std::vector<uint32_t>, std::vector<float>> make_cube_mesh()
    {
        std::vector<uint32_t> indexes;
        std::vector<float> vertexes;

        using Builder2F = Xyz::MeshAttributeBuilder<Xyz::Vector2F, std::vector<float>>;
        using Builder3F = Xyz::MeshAttributeBuilder<Xyz::Vector3F, std::vector<float>>;

        Xyz::MeshBuilder builder{
            .indexes = Xyz::MeshIndexBuilder(indexes),
            .coords = Builder3F(vertexes, STRIDE),
            .normals = std::optional(Builder3F(vertexes, STRIDE, 3)),
            .tex_coords = std::optional(Builder2F(vertexes, STRIDE, 6))
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

    // Named by both the mesh and the pipeline: a pipeline pins how the
    // attributes are packed, so the two have to agree.
    VertexLayoutRef make_layout(ResourceManager& resources)
    {
        return resources.register_layout(
            VertexLayoutBuilder()
                .add_attribute(AttributeSemantic::POSITION)
                .add_attribute(AttributeSemantic::NORMAL)
                .add_attribute(AttributeSemantic::TEX_COORD_0)
                .build());
    }

    // Both cubes draw through one pipeline: they differ only in their
    // material parameters and textures, which is material state, not
    // pipeline state.
    PipelineRef make_pipeline(ResourceManager& resources,
                              VertexLayoutRef layout)
    {
        PipelineDescriptor pipeline;
        pipeline.shader = resources.register_shader_variant(
            {.family = BLINN_PHONG_FAMILY});
        pipeline.layout = layout;
        pipeline.raster.cull_enabled = true;
        return resources.register_pipeline(pipeline);
    }

    TextureRef add_normal_map(ModelBuilder& builder)
    {
        const auto pixels = make_stud_normal_map();
        // DATA, not COLOR: these texels are a tangent-space vector
        // field. Decoding them as sRGB would not dim the bumps, it
        // would bend every normal the wrong way.
        return builder.add_texture({
            .size = {NORMAL_MAP_SIZE, NORMAL_MAP_SIZE},
            .format = RGB_TEXTURE,
            .content = TextureContent::DATA,
            .pixels = pixels.data(),
            .sampler = builder.resources().register_sampler({
                .mip_filter = SamplerMipFilter::NONE,
                .address_mode_u = SamplerAddressMode::CLAMP_TO_EDGE,
                .address_mode_v = SamplerAddressMode::CLAMP_TO_EDGE
            })
        });
    }

    /**
     * @param strength what goes in u_ambient.w: zero means the material has
     *      no normal map and the shader skips the sampler entirely.
     * @param normal_map the map to attach, if any.
     */
    MaterialRef add_material(ModelBuilder& builder, PipelineRef pipeline,
                             float strength, TextureRef normal_map = {})
    {
        Material material;
        material.pipeline = pipeline;
        material.parameter_data = make_blinn_phong_material_params(
            StandardColorMaterial::COPPER, 1.0f, strength);
        // The renderer binds textures to consecutive units, so the normal
        // map can only reach unit 2 behind two others. Null refs get the
        // renderer's white fallback, which leaves the material's own
        // colours unmodulated.
        if (normal_map)
            material.textures = {{}, {}, normal_map};
        return builder.add_material(std::move(material));
    }

    void add_cube(ModelBuilder& builder, ModelMeshId mesh,
                  MaterialRef material, float x)
    {
        auto node = builder.scene().add_node(builder.root());
        node.set_local_transform({.translation = {x, 0, 0}});
        builder.add_renderable(node.id(), mesh, material);
    }
}

Tungsten::Model add_bump_cube_model(Tungsten::ResourceManager& resources,
                                    Tungsten::Scene& scene,
                                    Tungsten::NodeId parent)
{
    ModelBuilder builder(resources, scene, parent);
    const auto layout = make_layout(resources);
    const auto pipeline = make_pipeline(resources, layout);

    // Same colour material for both, so every visible difference comes
    // from the normal map.
    const auto bumped = add_material(builder, pipeline, 1.0f,
                                     add_normal_map(builder));
    const auto plain = add_material(builder, pipeline, 0.0f);

    auto [indexes, vertexes] = make_cube_mesh();
    const auto mesh = builder.add_mesh(builder.add_vertices(layout, vertexes),
                                       std::move(indexes));
    add_cube(builder, mesh, bumped, -HALF_GAP);
    add_cube(builder, mesh, plain, HALF_GAP);
    return builder.build();
}
