//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-14.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "InfiniteGrid.hpp"

#include "EmbeddedResources.hpp"
#include "Tungsten/Color.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/VertexLayoutBuilder.hpp"

namespace Tungsten
{
    namespace
    {
        constexpr uint32_t VERTEX_COUNT = 4;
        constexpr uint32_t INDEX_COUNT = 6;

        // The width of the grid lines as a fraction of a cell.
        constexpr float LINE_WIDTH = 0.01f;
        // The width of the lines along the axes.
        constexpr float AXIS_WIDTH = 0.05f;
        // The number of cells per world unit.
        constexpr float GRID_SCALE = 1.0f;

        void register_shader_families(ResourceManager& resources)
        {
            ShaderFamily grid;
            grid.vertex_source = GRID_VERTEX;
            grid.fragment_source = GRID_FRAGMENT;
            grid.required_attributes =
                semantic_bit(AttributeSemantic::POSITION);
            resources.register_shader_family(GRID_FAMILY, std::move(grid));

            ShaderFamily axis;
            axis.vertex_source = AXIS_VERTEX;
            axis.fragment_source = AXIS_FRAGMENT;
            axis.required_attributes =
                semantic_bit(AttributeSemantic::POSITION);
            resources.register_shader_family(AXIS_FAMILY, std::move(axis));
        }

        // The corners are two-dimensional as the shader places them in the
        // xz plane. Named by both the mesh and the pipeline, which have to
        // agree on the packing.
        VertexLayoutRef grid_layout(ResourceManager& resources)
        {
            return resources.register_layout(
                VertexLayoutBuilder()
                .add_attribute(AttributeSemantic::POSITION)
                .set_component_count(2)
                .build());
        }

        MeshRef make_square_mesh(ResourceManager& resources,
                                 BufferArenaRef vbo_arena,
                                 BufferArenaRef ebo_arena)
        {
            constexpr float vertexes[2 * VERTEX_COUNT] = {
                -1, -1,
                1, -1,
                1, 1,
                -1, 1
            };
            uint16_t indexes[INDEX_COUNT] = {
                0, 1, 2,
                0, 2, 3
            };

            const auto vertices = resources.allocate(vbo_arena, VERTEX_COUNT);
            const auto indices = resources.allocate(ebo_arena, INDEX_COUNT);

            // Rebase to absolute indices: the portable path has no
            // baseVertex draw argument.
            for (auto& index : indexes)
                index = uint16_t(index + vertices.offset);

            resources.upload(vertices, vertexes, sizeof(vertexes));
            resources.upload(indices, indexes, sizeof(indexes));

            Mesh mesh;
            mesh.streams = {vertices};
            mesh.layout = grid_layout(resources);
            mesh.ebo = indices;
            return resources.create_mesh(std::move(mesh));
        }

        MaterialRef make_grid_material(ResourceManager& resources)
        {
            // No culling: the grid is as visible from below as from above.
            PipelineDescriptor descriptor;
            descriptor.shader = resources.register_shader_variant({GRID_FAMILY, 0});
            descriptor.layout = grid_layout(resources);
            descriptor.raster.cull_enabled = false;
            const auto pipeline = resources.register_pipeline(descriptor);

            // The shader's MaterialBlock: six vec4s. Colours in a uniform
            // block are linear; these are picked by eye, hence sRGB.
            const auto lines = srgb_to_linear(
                Xyz::Vector4F{0.85f, 0.85f, 0.85f, 1.0f});
            const auto base = srgb_to_linear(
                Xyz::Vector4F{0.16f, 0.17f, 0.19f, 1.0f});
            const auto x_axis = srgb_to_linear(
                Xyz::Vector4F{0.90f, 0.22f, 0.25f, 1.0f});
            const auto z_axis = srgb_to_linear(
                Xyz::Vector4F{0.25f, 0.45f, 0.95f, 1.0f});
            // The square has to cover everything in front of the far plane
            // wherever the camera looks, and the corners of the far plane are
            // further away than FAR_PLANE.
            const float values[24] = {
                lines[0], lines[1], lines[2], lines[3],
                base[0], base[1], base[2], base[3],
                x_axis[0], x_axis[1], x_axis[2], x_axis[3],
                z_axis[0], z_axis[1], z_axis[2], z_axis[3],
                LINE_WIDTH, LINE_WIDTH, GRID_SCALE, 2 * FAR_PLANE,
                AXIS_WIDTH, 0, 0, 0
            };
            const auto bytes = std::as_bytes(std::span(values));
            Material material;
            material.pipeline = pipeline;
            material.parameter_data = {bytes.begin(), bytes.end()};
            return resources.create_material(std::move(material));
        }

        MaterialRef make_axis_material(ResourceManager& resources)
        {
            // The line's edges are anti-aliased with alpha, so it is blended
            // and drawn after the grid. It doesn't write depth as its quad is
            // wider than the line.
            PipelineDescriptor descriptor;
            descriptor.shader =
                resources.register_shader_variant({AXIS_FAMILY, 0});
            descriptor.layout = grid_layout(resources);
            descriptor.queue = RenderQueue::TRANSPARENT;
            descriptor.depth.write = false;
            descriptor.blend.enabled = true;
            descriptor.blend.src_color = BlendFunction::SRC_ALPHA;
            descriptor.blend.dst_color = BlendFunction::ONE_MINUS_SRC_ALPHA;
            descriptor.blend.src_alpha = BlendFunction::ONE;
            descriptor.blend.dst_alpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
            descriptor.raster.cull_enabled = false;

            // The shader's MaterialBlock: two vec4s. The line is as long as
            // the grid is wide, which is past the far plane.
            const auto color = srgb_to_linear(
                Xyz::Vector4F{0.35f, 0.80f, 0.30f, 1.0f});
            const float values[8] = {
                color[0], color[1], color[2], color[3],
                AXIS_WIDTH, GRID_SCALE, 2 * FAR_PLANE, 0
            };
            const auto bytes = std::as_bytes(std::span(values));
            Material material;
            material.pipeline = resources.register_pipeline(descriptor);
            material.parameter_data = {bytes.begin(), bytes.end()};
            return resources.create_material(std::move(material));
        }
    }

    InfiniteGridResources::InfiniteGridResources(ResourceManager& resources,
                                                 BufferArenaRef vbo_arena,
                                                 BufferArenaRef ebo_arena)
        : resources_(resources)
    {
        register_shader_families(resources);

        if (!vbo_arena)
        {
            vbo_arena_ = vbo_arena = resources.create_arena(
                             BufferUsage::STATIC_DRAW, 2 * sizeof(float), VERTEX_COUNT);
        }
        if (!ebo_arena)
        {
            ebo_arena_ = ebo_arena = resources.create_arena(
                             BufferUsage::STATIC_DRAW, sizeof(uint16_t), INDEX_COUNT);
        }

        square_mesh_ = make_square_mesh(resources, vbo_arena, ebo_arena);
        grid_material_ = make_grid_material(resources);
        axis_material_ = make_axis_material(resources);
    }

    InfiniteGridResources::~InfiniteGridResources()
    {
        resources_.destroy_mesh(square_mesh_);
        resources_.destroy_material(axis_material_);
        resources_.destroy_material(grid_material_);

        if (vbo_arena_)
            resources_.destroy_arena(vbo_arena_);
        if (ebo_arena_)
            resources_.destroy_arena(ebo_arena_);
    }

    NodeHandle
    make_infinite_grid_node(Scene& scene, const InfiniteGridResources& resources)
    {
        auto hub = scene.add_node();

        hub.add_child().add(RenderableComponent{
            .mesh = resources.square_mesh(),
            .material = resources.grid_material()
        });

        hub.add_child().add(RenderableComponent{
            .mesh = resources.square_mesh(),
            .material = resources.axis_material()
        });

        return hub;
    }
} // Tungsten
