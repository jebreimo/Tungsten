//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <vector>
#include "Model.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/Texture.hpp"
#include "../SceneGraph/Skin.hpp"

namespace Tungsten
{
    /** Identifies vertices queued with ModelBuilder::add_vertices. */
    using ModelVerticesId = uint32_t;

    /** Identifies a mesh queued with ModelBuilder::add_mesh. */
    using ModelMeshId = uint32_t;

    /**
     * @brief Builds a Model: creates its root node, and creates and keeps
     *      track of the resources its renderables need.
     *
     * Vertices, meshes and renderables are queued and only created by
     * build(), which then knows how much of each kind of buffer the model
     * needs. It allocates one buffer arena per vertex size and one for the
     * indices, sized so that they never have to grow, and gives the queued
     * renderables their meshes. Materials and textures are created at once.
     *
     * If the builder is destroyed before build() is called, everything it
     * has added to the scene and the resource manager is removed again, so
     * a model that fails halfway leaves nothing behind.
     */
    class ModelBuilder
    {
    public:
        /**
         * Creates the model's root node as a child of @a parent.
         */
        ModelBuilder(ResourceManager& resources, Scene& scene,
                     NodeId parent = {});

        ModelBuilder(const ModelBuilder&) = delete;

        ModelBuilder& operator=(const ModelBuilder&) = delete;

        ~ModelBuilder();

        [[nodiscard]]
        ResourceManager& resources() const;

        [[nodiscard]]
        Scene& scene() const;

        [[nodiscard]]
        NodeId root() const;

        /**
         * Queues vertices for upload.
         *
         * The vertices are packed as @a layout describes, which must have a
         * single stream and a POSITION attribute of at least three floats.
         * The number of vertices is the size of @a data divided by the size
         * of one vertex.
         */
        ModelVerticesId add_vertices(VertexLayoutRef layout,
                                     std::span<const std::byte> data);

        /**
         * Queues the elements of @a data, a contiguous range of floats,
         * integers or a struct of them, as vertices.
         */
        template <std::ranges::contiguous_range Range>
        ModelVerticesId add_vertices(VertexLayoutRef layout, const Range& data)
        {
            return add_vertices(layout, std::as_bytes(std::span(data)));
        }

        /**
         * Queues a mesh that draws @a vertices through @a indices, which
         * count from the first of those vertices. Several meshes can share
         * the same vertices.
         */
        ModelMeshId add_mesh(ModelVerticesId vertices,
                             std::vector<uint32_t> indices);

        /**
         * Queues a RenderableComponent for @a node that draws @a mesh with
         * @a material. The node must be in the root's subtree, or it is not
         * removed with the model.
         *
         * Its local bounds are those of the mesh's vertices, except when
         * @a skin is set: the joints can move a skinned mesh anywhere, so it
         * is never culled. The model's bounds include it in its bind pose.
         */
        void add_renderable(NodeId node, ModelMeshId mesh,
                            MaterialRef material,
                            std::shared_ptr<const Skin> skin = {});

        /**
         * Creates a material that is destroyed with the model.
         */
        MaterialRef add_material(Material material);

        /**
         * Creates a texture that is destroyed with the model.
         */
        TextureRef add_texture(const TextureImage2D& image);

        /**
         * Uploads everything that has been queued, gives the queued
         * renderables their components and returns the finished model.
         *
         * Resolves the scene's transforms to measure the model's bounds.
         * The builder must not be used afterwards, except for get_mesh.
         */
        Model build();

        /**
         * The mesh build() created for @a mesh.
         */
        [[nodiscard]]
        MeshRef get_mesh(ModelMeshId mesh) const;

    private:
        struct QueuedVertices
        {
            VertexLayoutRef layout;
            uint16_t stride = 0;
            uint32_t count = 0;
            std::vector<std::byte> data;
            Xyz::BBox3F bounds;
        };

        struct QueuedMesh
        {
            ModelVerticesId vertices = 0;
            std::vector<uint32_t> indices;
        };

        struct QueuedRenderable
        {
            NodeId node;
            ModelMeshId mesh = 0;
            MaterialRef material;
            std::shared_ptr<const Skin> skin;
        };

        void create_arenas();

        void create_meshes();

        void add_renderables();

        ResourceManager& resources_;
        Scene& scene_;
        Model model_;
        std::vector<QueuedVertices> vertices_;
        std::vector<QueuedMesh> meshes_;
        std::vector<QueuedRenderable> renderables_;
        std::vector<SharedBuffer> vertex_slices_;
        std::vector<MeshRef> mesh_refs_;
        std::vector<std::pair<uint16_t, BufferArenaRef>> vertex_arenas_;
        BufferArenaRef index_arena_;
        bool built_ = false;
    };
} // Tungsten
