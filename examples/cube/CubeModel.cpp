//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "CubeModel.hpp"

#include <Tungsten/Tungsten.hpp>

#include "EmbeddedResources.hpp"

namespace
{
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

        auto get_tex_rect = [](int i)
        {
            return Xyz::RectangleF{{float(i) / 6, 1}, {1.f / 6, -1}};
        };

        auto cuboid = Xyz::make_centered_oriented_cuboid<float>(
            {0, 0, 0},
            {2, 2, 2},
            {-Xyz::to_radians(-90.f), -Xyz::to_radians(-90.f), 0}
        );
        Xyz::build_mesh(builder, cuboid, std::function(get_tex_rect));
        return {std::move(indexes), std::move(vertexes)};
    }

    // Named by both the mesh and the pipeline: a pipeline pins how the
    // attributes are packed, so the two have to agree.
    Tungsten::VertexLayoutRef make_layout(Tungsten::ResourceManager& resources)
    {
        return resources.register_layout(
            Tungsten::VertexLayoutBuilder()
                .add_attribute(Tungsten::AttributeSemantic::POSITION)
                .add_attribute(Tungsten::AttributeSemantic::NORMAL)
                .add_attribute(Tungsten::AttributeSemantic::TEX_COORD_0)
                .build());
    }

    // Everything fixed before the draw call — the program, the vertex
    // input and the fixed-function state — registered once as a pipeline.
    Tungsten::PipelineRef make_pipeline(Tungsten::ResourceManager& resources,
                                        Tungsten::VertexLayoutRef layout)
    {
        Tungsten::PipelineDescriptor pipeline;
        pipeline.shader = resources.register_shader_variant(
            {.family = Tungsten::BLINN_PHONG_FAMILY});
        pipeline.layout = layout;
        pipeline.raster.cull_enabled = true;
        return resources.register_pipeline(pipeline);
    }

    Tungsten::TextureRef add_texture(Tungsten::ModelBuilder& builder)
    {
        const auto image = Tungsten::read_image(NUMBERS_PNG, NUMBERS_PNG_SIZE);

        // COLOR: an image file's pixels are sRGB, so the sampler decodes
        // them and the shader gets linear values to light. How the texture
        // is sampled is a property of the Texture resource, not state baked
        // into the GL object: the renderer binds this sampler to the unit
        // the texture lands on.
        return builder.add_texture({
            .size = {int32_t(image.width()), int32_t(image.height())},
            .format = Tungsten::get_ogl_pixel_type(image.pixel_type()),
            .content = Tungsten::TextureContent::COLOR,
            .pixels = image.data(),
            .sampler = builder.resources().register_sampler(
                {
                    .mip_filter = Tungsten::SamplerMipFilter::NONE,
                    .address_mode_u = Tungsten::SamplerAddressMode::CLAMP_TO_EDGE,
                    .address_mode_v = Tungsten::SamplerAddressMode::CLAMP_TO_EDGE
                })
        });
    }
}

Tungsten::Model add_cube_model(Tungsten::ResourceManager& resources,
                               Tungsten::Scene& scene,
                               Tungsten::NodeId parent)
{
    Tungsten::ModelBuilder builder(resources, scene, parent);
    const auto layout = make_layout(resources);

    Tungsten::Material gold;
    gold.pipeline = make_pipeline(resources, layout);
    gold.parameter_data = Tungsten::make_blinn_phong_material_params(
        Tungsten::StandardColorMaterial::GOLD, 1.0f);
    gold.textures = {add_texture(builder)};

    auto [indexes, vertexes] = make_cube_mesh();
    const auto mesh = builder.add_mesh(builder.add_vertices(layout, vertexes),
                                       std::move(indexes));
    builder.add_renderable(builder.root(), mesh,
                           builder.add_material(std::move(gold)));
    return builder.build();
}
