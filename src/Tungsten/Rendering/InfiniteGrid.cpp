//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-14.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Rendering/InfiniteGrid.hpp"

#include <span>
#include "Tungsten/Color.hpp"
#include "Tungsten/Resources/BuiltinShaders.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/Resources/Shaders/BuiltinShaderSources.hpp"
#include "Tungsten/Resources/VertexLayoutBuilder.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"

namespace Tungsten
{
    namespace
    {
        constexpr uint32_t VERTEX_COUNT = 4;
        constexpr uint32_t INDEX_COUNT = 6;

        // The MaterialBlock of InfiniteGrid-*.glsl (std140).
        struct GridBlock
        {
            Xyz::Vector4F line_color;
            Xyz::Vector4F base_color;
            Xyz::Vector4F x_axis_color;
            Xyz::Vector4F z_axis_color;
            // x = line width, y = axis width, z = cells per world unit.
            Xyz::Vector4F grid;
        };

        static_assert(sizeof(GridBlock) == 5 * 16);

        // The MaterialBlock of GridAxis-*.glsl (std140).
        struct AxisBlock
        {
            Xyz::Vector4F color;
            // x = line width, y = cells per world unit.
            Xyz::Vector4F axis;
        };

        static_assert(sizeof(AxisBlock) == 2 * 16);

        template <typename T>
        std::vector<std::byte> to_bytes(const T& block)
        {
            const auto bytes = std::as_bytes(std::span(&block, 1));
            return {bytes.begin(), bytes.end()};
        }

        void register_shader_families(ResourceManager& resources)
        {
            ShaderFamily grid;
            grid.vertex_source = INFINITE_GRID_VERTEX;
            grid.fragment_source = INFINITE_GRID_FRAGMENT;
            grid.required_attributes =
                semantic_bit(AttributeSemantic::POSITION);
            resources.register_shader_family(INFINITE_GRID_FAMILY,
                                             std::move(grid));

            ShaderFamily axis;
            axis.vertex_source = GRID_AXIS_VERTEX;
            axis.fragment_source = GRID_AXIS_FRAGMENT;
            axis.required_attributes =
                semantic_bit(AttributeSemantic::POSITION);
            resources.register_shader_family(GRID_AXIS_FAMILY,
                                             std::move(axis));
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

        MaterialRef make_grid_material(ResourceManager& resources,
                                       const InfiniteGridStyle& style)
        {
            // No culling: the grid is as visible from below as from above.
            PipelineDescriptor descriptor;
            descriptor.shader = resources.register_shader_variant(
                {INFINITE_GRID_FAMILY, 0});
            descriptor.layout = grid_layout(resources);
            descriptor.raster.cull_enabled = false;

            // Colours in a uniform block are linear.
            const GridBlock block = {
                .line_color = srgb_to_linear(style.line_color),
                .base_color = srgb_to_linear(style.base_color),
                .x_axis_color = srgb_to_linear(style.x_axis_color),
                .z_axis_color = srgb_to_linear(style.z_axis_color),
                .grid = {style.line_width, style.axis_width,
                         style.cells_per_unit, 0}
            };

            Material material;
            material.pipeline = resources.register_pipeline(descriptor);
            material.parameter_data = to_bytes(block);
            return resources.create_material(std::move(material));
        }

        MaterialRef make_axis_material(ResourceManager& resources,
                                       const InfiniteGridStyle& style)
        {
            // The line's edges are anti-aliased with alpha, so it is blended
            // and drawn after the grid. It doesn't write depth as its quad is
            // wider than the line.
            PipelineDescriptor descriptor;
            descriptor.shader = resources.register_shader_variant(
                {GRID_AXIS_FAMILY, 0});
            descriptor.layout = grid_layout(resources);
            descriptor.queue = RenderQueue::TRANSPARENT;
            descriptor.depth.write = false;
            descriptor.blend.enabled = true;
            descriptor.blend.src_color = BlendFunction::SRC_ALPHA;
            descriptor.blend.dst_color = BlendFunction::ONE_MINUS_SRC_ALPHA;
            descriptor.blend.src_alpha = BlendFunction::ONE;
            descriptor.blend.dst_alpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
            descriptor.raster.cull_enabled = false;

            const AxisBlock block = {
                .color = srgb_to_linear(style.y_axis_color),
                .axis = {style.axis_width, style.cells_per_unit, 0, 0}
            };

            Material material;
            material.pipeline = resources.register_pipeline(descriptor);
            material.parameter_data = to_bytes(block);
            return resources.create_material(std::move(material));
        }
    }

    InfiniteGrid::InfiniteGrid(ResourceManager& resources,
                               const InfiniteGridStyle& style)
        : resources_(resources)
    {
        register_shader_families(resources);

        vbo_arena_ = resources.create_arena(
            BufferUsage::STATIC_DRAW, 2 * sizeof(float), VERTEX_COUNT);
        ebo_arena_ = resources.create_arena(
            BufferUsage::STATIC_DRAW, sizeof(uint16_t), INDEX_COUNT);

        square_mesh_ = make_square_mesh(resources, vbo_arena_, ebo_arena_);
        grid_material_ = make_grid_material(resources, style);
        if (style.show_y_axis)
            axis_material_ = make_axis_material(resources, style);
    }

    InfiniteGrid::~InfiniteGrid()
    {
        resources_.destroy_mesh(square_mesh_);
        if (axis_material_)
            resources_.destroy_material(axis_material_);
        resources_.destroy_material(grid_material_);
        resources_.destroy_arena(vbo_arena_);
        resources_.destroy_arena(ebo_arena_);
    }

    NodeHandle InfiniteGrid::add_to(Scene& scene) const
    {
        auto hub = scene.add_node();

        hub.add_child().add(RenderableComponent{
            .mesh = square_mesh_,
            .material = grid_material_
        });

        if (axis_material_)
        {
            hub.add_child().add(RenderableComponent{
                .mesh = square_mesh_,
                .material = axis_material_
            });
        }

        return hub;
    }
} // Tungsten
