//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Model/ModelBuilder.hpp"

#include <algorithm>
#include <bit>
#include <cstring>
#include <string>
#include <Xyz/InvertMatrix.hpp>
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/Resources/VertexLayout.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"

namespace Tungsten
{
    namespace
    {
        /**
         * The capacity an arena needs to hold allocations of @a sizes without
         * ever growing.
         *
         * The arenas are sub-allocated by a buddy allocator, which rounds
         * every allocation up to a power of two, so the sum of the sizes is
         * not enough — the sum of the rounded sizes is. Sizing for the whole
         * model up front is why meshes are queued: growing an arena copies
         * the entire buffer through glCopyBufferSubData, which macOS's GL
         * driver reports as unimplemented.
         */
        uint32_t required_capacity(const std::vector<uint32_t>& sizes)
        {
            // A lone allocation lands at offset 0 and needs no rounding.
            if (sizes.size() == 1)
                return std::max(sizes[0], 1u);

            uint32_t total = 0;
            for (const auto size : sizes)
                total += std::bit_ceil(std::max(size, 1u));
            return std::bit_ceil(std::max(total, 1u));
        }

        /**
         * The size of one vertex in a single-stream layout.
         */
        uint16_t get_stride(const VertexLayout& layout)
        {
            uint32_t stride = 0;
            for (const auto& attribute : layout.attributes)
            {
                if (attribute.stream_index != 0)
                    TUNGSTEN_THROW("Model vertices must have a single stream.");
                stride = std::max(stride, attribute.offset_in_stream
                                          + byte_size(attribute));
            }
            if (stride == 0)
                TUNGSTEN_THROW("Model vertices must have attributes.");
            return uint16_t(stride);
        }

        Xyz::BBox3F get_bounds(const VertexLayout& layout,
                               std::span<const std::byte> data,
                               uint16_t stride)
        {
            const auto it = std::ranges::find(layout.attributes,
                                              AttributeSemantic::POSITION,
                                              &VertexAttribute::semantic);
            if (it == layout.attributes.end()
                || it->data_type != VertexAttributeDataType::FLOAT
                || it->component_count < 3)
            {
                TUNGSTEN_THROW("Model vertices must have a POSITION attribute"
                               " of at least three floats.");
            }

            Xyz::BBox3F bounds;
            for (size_t offset = it->offset_in_stream;
                 offset + 3 * sizeof(float) <= data.size();
                 offset += stride)
            {
                // The data need not be aligned for floats.
                Xyz::Vector3F position;
                std::memcpy(&position[0], data.data() + offset, sizeof(float));
                std::memcpy(&position[1], data.data() + offset + 4, sizeof(float));
                std::memcpy(&position[2], data.data() + offset + 8, sizeof(float));
                bounds += position;
            }
            return bounds;
        }
    }

    ModelBuilder::ModelBuilder(ResourceManager& resources, Scene& scene,
                               NodeId parent)
        : resources_(resources),
          scene_(scene)
    {
        model_.root = scene_.create_node(parent);
    }

    ModelBuilder::~ModelBuilder()
    {
        if (!built_)
            remove_model(model_, scene_, resources_);
    }

    ResourceManager& ModelBuilder::resources() const
    {
        return resources_;
    }

    Scene& ModelBuilder::scene() const
    {
        return scene_;
    }

    NodeId ModelBuilder::root() const
    {
        return model_.root;
    }

    ModelVerticesId ModelBuilder::add_vertices(VertexLayoutRef layout,
                                               std::span<const std::byte> data)
    {
        const auto& vertex_layout = resources_.get_layout(layout);
        const auto stride = get_stride(vertex_layout);
        if (data.size() % stride != 0)
        {
            TUNGSTEN_THROW("" + std::to_string(data.size()) + " bytes is not a"
                           " whole number of " + std::to_string(stride)
                           + "-byte vertices.");
        }

        vertices_.push_back({
            .layout = layout,
            .stride = stride,
            .count = uint32_t(data.size() / stride),
            .data = {data.begin(), data.end()},
            .bounds = get_bounds(vertex_layout, data, stride)
        });
        return ModelVerticesId(vertices_.size() - 1);
    }

    ModelMeshId ModelBuilder::add_mesh(ModelVerticesId vertices,
                                       std::vector<uint32_t> indices)
    {
        if (vertices >= vertices_.size())
            TUNGSTEN_THROW("No such model vertices.");
        if (indices.empty())
            TUNGSTEN_THROW("A mesh must have indices.");
        const auto count = vertices_[vertices].count;
        if (std::ranges::any_of(indices, [&](uint32_t i) { return i >= count; }))
            TUNGSTEN_THROW("A mesh index is past the end of its vertices.");

        meshes_.push_back({vertices, std::move(indices)});
        return ModelMeshId(meshes_.size() - 1);
    }

    void ModelBuilder::add_renderable(NodeId node, ModelMeshId mesh,
                                      MaterialRef material,
                                      std::shared_ptr<const Skin> skin)
    {
        if (mesh >= meshes_.size())
            TUNGSTEN_THROW("No such model mesh.");
        renderables_.push_back({node, mesh, material, std::move(skin)});
    }

    MaterialRef ModelBuilder::add_material(Material material)
    {
        const auto ref = resources_.create_material(std::move(material));
        model_.resources.materials.push_back(ref);
        return ref;
    }

    TextureRef ModelBuilder::add_texture(const TextureImage2D& image)
    {
        const auto ref = resources_.create_texture(image);
        model_.resources.textures.push_back(ref);
        return ref;
    }

    Model ModelBuilder::build()
    {
        if (built_)
            TUNGSTEN_THROW("The model has already been built.");

        // Exceptions leave built_ unset, so the destructor cleans up.
        create_arenas();
        create_meshes();
        add_renderables();

        // World matrices are what the bounds have to be measured in, and
        // they are only valid after a resolve.
        scene_.resolve_transforms();
        const auto world_to_root = Xyz::invert(scene_.world_matrix(model_.root));
        for (const auto& renderable : renderables_)
        {
            const auto& vertices = vertices_[meshes_[renderable.mesh].vertices];
            model_.bounds += Xyz::transform_bbox(
                vertices.bounds,
                world_to_root * scene_.world_matrix(renderable.node));
        }

        vertices_.clear();
        meshes_.clear();
        renderables_.clear();
        built_ = true;
        return std::move(model_);
    }

    MeshRef ModelBuilder::get_mesh(ModelMeshId mesh) const
    {
        if (mesh >= mesh_refs_.size())
            TUNGSTEN_THROW("No such model mesh, or the model is not built.");
        return mesh_refs_[mesh];
    }

    void ModelBuilder::create_arenas()
    {
        // An arena holds one size of vertex.
        std::vector<std::pair<uint16_t, std::vector<uint32_t>>> vertex_sizes;
        for (const auto& vertices : vertices_)
        {
            auto it = std::ranges::find(vertex_sizes, vertices.stride,
                                        &std::pair<uint16_t, std::vector<uint32_t>>::first);
            if (it == vertex_sizes.end())
                it = vertex_sizes.insert(it, {vertices.stride, {}});
            it->second.push_back(vertices.count);
        }

        // The model's own arenas, so its slices are freed with them and need
        // not be listed in the bundle.
        for (const auto& [stride, sizes] : vertex_sizes)
        {
            const auto arena = resources_.create_arena(
                BufferUsage::STATIC_DRAW, stride, required_capacity(sizes));
            model_.resources.arenas.push_back(arena);
            vertex_arenas_.emplace_back(stride, arena);
        }

        if (meshes_.empty())
            return;

        std::vector<uint32_t> index_sizes;
        index_sizes.reserve(meshes_.size());
        for (const auto& mesh : meshes_)
            index_sizes.push_back(uint32_t(mesh.indices.size()));
        index_arena_ = resources_.create_arena(
            BufferUsage::STATIC_DRAW, sizeof(uint32_t),
            required_capacity(index_sizes));
        model_.resources.arenas.push_back(index_arena_);
    }

    void ModelBuilder::create_meshes()
    {
        for (const auto& vertices : vertices_)
        {
            const auto arena = std::ranges::find(
                vertex_arenas_, vertices.stride,
                &std::pair<uint16_t, BufferArenaRef>::first)->second;
            const auto slice = resources_.allocate(arena, vertices.count);
            resources_.upload(slice, vertices.data.data(), vertices.data.size());
            vertex_slices_.push_back(slice);
        }

        for (auto& queued : meshes_)
        {
            const auto& vertices = vertex_slices_[queued.vertices];
            const auto ebo = resources_.allocate(
                index_arena_, uint32_t(queued.indices.size()));

            // Rebase to absolute indices: there is no baseVertex draw
            // argument, and all the model's vertices of one size share an
            // arena, so 16-bit indices would not get far.
            for (auto& index : queued.indices)
                index += vertices.offset;
            resources_.upload(ebo, queued.indices.data(),
                              queued.indices.size() * sizeof(uint32_t));

            Mesh mesh;
            mesh.streams = {vertices};
            mesh.layout = vertices_[queued.vertices].layout;
            mesh.ebo = ebo;
            mesh.index_type = ElementIndexType::UINT32;
            const auto ref = resources_.create_mesh(std::move(mesh));
            model_.resources.meshes.push_back(ref);
            mesh_refs_.push_back(ref);
        }
    }

    void ModelBuilder::add_renderables()
    {
        for (const auto& renderable : renderables_)
        {
            const auto& vertices = vertices_[meshes_[renderable.mesh].vertices];
            scene_.add_component(renderable.node, RenderableComponent{
                .mesh = mesh_refs_[renderable.mesh],
                .material = renderable.material,
                .local_bounds = renderable.skin ? Xyz::BBox3F() : vertices.bounds,
                .skin = renderable.skin
            });
        }
    }
} // Tungsten
