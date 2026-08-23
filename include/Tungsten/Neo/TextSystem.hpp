//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>
#include <Xyz/Rectangle.hpp>
#include <Xyz/Vector.hpp>
#include "NodeId.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    struct Font;
    class ResourceManager;
    class Scene;
    struct TextComponent;
    struct TextStyle;

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
        /**
         * Brings one component up to date: rebuilds its geometry and material
         * as needed, then writes the node's RenderableComponent.
         */
        void update_component(Scene& scene, NodeId owner, TextComponent& text);

        /**
         * Re-tessellates a component's glyph quads and uploads them into fresh
         * slices of the arenas, freeing the ones it held. Text that has become
         * empty releases its mesh entirely rather than keeping a zero-length
         * one.
         */
        void rebuild_geometry(TextComponent& text);

        /**
         * The material for a font's atlas and an effective colour, created on
         * first use of that pair.
         */
        MaterialRef resolve_material(const TextStyle& style,
                                     const std::optional<Xyz::Vector4F>& color);

        /**
         * The atlas texture for a font, uploaded on first use.
         */
        TextureRef get_atlas(const std::shared_ptr<const Font>& font);

        /**
         * Takes a slot in the entry table, reusing a released one when there
         * is one.
         */
        uint32_t acquire_slot();

        /**
         * Destroys the entry's mesh — which returns its slices to the arenas —
         * and frees the slot.
         */
        void release_slot(uint32_t slot);

        ResourceManager& resources_;
        BufferArenaRef vertex_arena_;
        BufferArenaRef index_arena_;
        VertexLayoutRef layout_;
        ShaderProgramRef shader_;

        struct Atlas
        {
            std::shared_ptr<const Font> font;
            TextureRef texture;
        };

        struct MaterialEntry
        {
            TextureRef atlas;
            Xyz::Vector4F color;
            MaterialRef material;
        };

        /**
         * One entry per live text item, indexed by TextComponent's
         * BuiltState::slot. Entries are never shifted — a released one goes on
         * free_slots_ instead — so the index a component stores stays valid.
         */
        struct Entry
        {
            MeshRef mesh;
        };

        /**
         * Both are searched linearly. The number of distinct fonts, and of
         * distinct (atlas, colour) pairs, is small and bounded in any real
         * application — the same assumption ShaderLibrary's variant cache
         * makes.
         */
        std::vector<Atlas> atlases_;
        std::vector<MaterialEntry> materials_;

        std::vector<Entry> entries_;
        std::vector<uint32_t> free_slots_;
        /**
         * Which entries a live component claimed during the current sweep.
         * Whatever is left unclaimed afterwards belongs to a component that
         * was removed, or whose node was, and is released. Kept as a member so
         * the per-frame sweep does not allocate.
         */
        std::vector<uint8_t> claimed_;

        /**
         * Tessellation scratch, reused across items and frames so that a
         * steady state — a handful of values changing each frame — does not
         * allocate. The glyph_* pair is what the font layout code fills (its
         * GlyphVertex is this tuple, spelled out to keep an internal header
         * out of this one); vertexes_ and indexes_ are the anchored, rebased
         * form that goes to the GPU.
         */
        std::vector<std::tuple<Xyz::Vector2F, Xyz::Vector2F>> glyph_vertexes_;
        std::vector<int32_t> glyph_indexes_;
        std::vector<TextVertex> vertexes_;
        std::vector<uint32_t> indexes_;
    };
} // Tungsten
