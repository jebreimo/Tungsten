//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
#include <memory>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    class ResourceManager;
    class Scene;

    /**
     * One corner of a glyph quad — the vertex format text_vertex_layout()
     * describes. Positions are 2D and in the font's own units (pixels at the
     * size the atlas was rasterized for); the node's transform takes them to
     * world space.
     */
    struct TextVertex
    {
        Xyz::Vector2F position;
        Xyz::Vector2F tex_coord;
    };

    static_assert(sizeof(TextVertex) == 16,
                  "TextVertex must match text_vertex_layout()'s stride.");

    /**
     * Compiles the scene's TextComponents into the meshes and materials the
     * Renderer draws. It is the only part of Neo that knows what a glyph is:
     * downstream, text is an ordinary textured, transparent renderable.
     *
     * Call update() once per frame, before Scene::resolve_transforms() and
     * SnapshotBuilder::build():
     *
     *     text_system.update(scene);
     *     scene.resolve_transforms();
     *     builder.build(scene, camera_node, snapshot);
     *     renderer.render(snapshot);
     *
     * What it owns, and why each is shared rather than per item:
     *
     * - **Two buffer arenas**, one for glyph vertices and one for indices, so
     *   every text item in the scene sub-allocates from the same pair of GL
     *   buffers (§7). Every text mesh therefore also shares one VAO, and
     *   re-uploading a changed item touches only its own slices.
     * - **One atlas Texture per Font**, created on first use and keyed on the
     *   font pointer.
     * - **One Material per (atlas, colour) pair.** Colour lives in the
     *   material's parameter block, not in the vertex data, so the hundred
     *   cells of a table sharing a style also share one material: the sort key
     *   groups them and the renderer binds it once. It also means a colour
     *   change costs no tessellation.
     *
     * Draw calls are still one per text item — Neo's per-draw UBO is what
     * gives each item its own transform, and merging items into one mesh would
     * mean giving that up, along with per-item culling. The batching that is
     * left, and that matters, is state batching: same VAO, same shader, and
     * one material bind per distinct colour.
     *
     * The constructor creates GL objects, so a context must be current — the
     * same requirement Renderer's constructor has.
     */
    class TextSystem
    {
    public:
        /**
         * Registers the text shader family and interns its vertex layout, then
         * creates the glyph vertex and index arenas.
         */
        explicit TextSystem(ResourceManager& resources);

        ~TextSystem();

        /**
         * The system holds refs into one ResourceManager and slots that
         * components point back at, so it is neither copied nor moved.
         */
        TextSystem(const TextSystem&) = delete;
        TextSystem& operator=(const TextSystem&) = delete;

        /**
         * Brings every TextComponent's published RenderableComponent up to
         * date, tessellating and uploading whatever changed since the last
         * call, and releases the meshes of components that have gone away.
         *
         * Requires a current GL context. Adds a RenderableComponent to a text
         * node that has none.
         */
        void update(Scene& scene);

        /**
         * The number of text items currently holding GPU memory. For tests and
         * for checking that removed text is actually being reclaimed.
         */
        [[nodiscard]]
        size_t live_item_count() const;

    private:
        struct Members;
        std::unique_ptr<Members> members_;
    };
} // Tungsten
