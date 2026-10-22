//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "cube_hub_model.hpp"

#include <Tungsten/Tungsten.hpp>
#include <Xyz/Mesh/BuildMesh.hpp>

namespace
{
    using namespace Tungsten;

    // 3 coords + 3 normals + 2 tex coords
    constexpr size_t STRIDE = 8;

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

        constexpr Xyz::OrientedCuboid<float> cuboid{{{-1, -1, -1}, {0, 0, 0}}, {2, 2, 2}};
        Xyz::build_mesh(builder, cuboid);
        return {std::move(indexes), std::move(vertexes)};
    }

    // The one vertex packing this example uses. Both the mesh and the
    // pipeline name it: a pipeline pins how attributes are packed, so the
    // two have to agree.
    VertexLayoutRef make_layout(ResourceManager& resources)
    {
        return resources.register_layout(VertexLayoutBuilder()
            .add_attribute(AttributeSemantic::POSITION)
            .add_attribute(AttributeSemantic::NORMAL)
            .add_attribute(AttributeSemantic::TEX_COORD_0)
            .build());
    }

    // Appearance that is fixed before the draw — the program, the vertex
    // input, and the depth/blend/raster state — is registered once as a
    // pipeline. A transparent one blends and leaves depth writes off, so
    // blended surfaces do not occlude each other.
    PipelineRef make_pipeline(ResourceManager& resources,
                              ShaderProgramRef shader,
                              VertexLayoutRef layout, bool transparent)
    {
        PipelineDescriptor pipeline;
        pipeline.shader = shader;
        pipeline.layout = layout;
        pipeline.raster.cull_enabled = true;
        if (transparent)
        {
            pipeline.queue = RenderQueue::TRANSPARENT;
            pipeline.depth.write = false;
            pipeline.blend = {
                .enabled = true,
                .src_color = BlendFunction::SRC_ALPHA,
                .dst_color = BlendFunction::ONE_MINUS_SRC_ALPHA,
                .src_alpha = BlendFunction::SRC_ALPHA,
                .dst_alpha = BlendFunction::ONE_MINUS_SRC_ALPHA
            };
        }
        return resources.register_pipeline(pipeline);
    }

    MaterialRef add_material(ModelBuilder& builder, PipelineRef pipeline,
                             std::vector<std::byte> params)
    {
        Material material;
        material.pipeline = pipeline;
        material.parameter_data = std::move(params);
        return builder.add_material(std::move(material));
    }

    NodeId add_child_cube(ModelBuilder& builder, ModelMeshId mesh,
                          MaterialRef material, float x)
    {
        auto child = builder.scene().add_node(builder.root());
        child.set_local_transform({
            .translation = {x, 0, 0},
            .scale = {CHILD_CUBE_SCALE, CHILD_CUBE_SCALE, CHILD_CUBE_SCALE}
        });
        builder.add_renderable(child.id(), mesh, material);
        return child.id();
    }
}

CubeHubModel add_cube_hub_model(Tungsten::ResourceManager& resources,
                                Tungsten::Scene& scene,
                                Tungsten::NodeId parent)
{
    ModelBuilder builder(resources, scene, parent);

    const auto shader = resources.register_shader_variant(
        {BLINN_PHONG_FAMILY, 0});
    const auto layout = make_layout(resources);
    const auto opaque = make_pipeline(resources, shader, layout, false);
    const auto blended = make_pipeline(resources, shader, layout, true);

    const auto gold = add_material(
        builder, opaque,
        make_blinn_phong_material_params(StandardColorMaterial::GOLD));
    const auto slate = add_material(
        builder, opaque,
        make_blinn_phong_material_params(StandardColorMaterial::SLATE));
    const auto glass = add_material(
        builder, blended,
        make_blinn_phong_material_params(StandardColorMaterial::COPPER,
                                         0.4f));

    auto [indexes, vertexes] = make_cube_mesh();
    const auto mesh = builder.add_mesh(builder.add_vertices(layout, vertexes),
                                       std::move(indexes));

    builder.add_renderable(builder.root(), mesh, gold);
    const auto left = add_child_cube(builder, mesh, glass, -CHILD_CUBE_OFFSET);
    const auto right = add_child_cube(builder, mesh, slate, CHILD_CUBE_OFFSET);
    return {builder.build(), left, right};
}
