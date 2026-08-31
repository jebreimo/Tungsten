//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/TextSystem.hpp"

#include <algorithm>
#include <cstring>
#include <optional>
#include <tuple>
#include <vector>
#include <Xyz/Rectangle.hpp>
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Neo/BuiltinShaders.hpp"
#include "Tungsten/Neo/Material.hpp"
#include "Tungsten/Neo/Mesh.hpp"
#include "Tungsten/Neo/NodeId.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/ResourceRefs.hpp"
#include "Tungsten/Neo/Scene.hpp"
#include "Tungsten/Neo/TextComponent.hpp"
#include "Tungsten/Neo/TextStyle.hpp"
#include "Tungsten/Neo/Texture.hpp"
#include "Tungsten/Neo/VertexLayoutBuilder.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/YimageGl.hpp"
#include "../Render/FontUtilities.hpp"
#include "../Render/TextUtilities.hpp"
#include "Shaders/BuiltinShaderSources.hpp"

namespace Tungsten
{
    namespace
    {
        // Sized for a few hundred short strings; both arenas double from here
        // as needed, and growth preserves every live slice's offset (§7).
        constexpr uint32_t INITIAL_VERTEX_CAPACITY = 4096; // 1024 glyphs
        constexpr uint32_t INITIAL_INDEX_CAPACITY = 8192;

        constexpr uint16_t VERTEX_STRIDE = uint16_t(sizeof(TextVertex));
        constexpr uint16_t INDEX_STRIDE = uint16_t(sizeof(uint32_t));

        void register_text_family(ResourceManager& resources)
        {
            ShaderFamily family;
            family.vertex_source = TEXT_VERTEX;
            family.fragment_source = TEXT_FRAGMENT;
            // The atlas is the material's only texture, so it lands on unit 0.
            family.samplers = {"u_text_atlas"};
            family.required_attributes =
                semantic_bit(AttributeSemantic::POSITION)
                | semantic_bit(AttributeSemantic::TEX_COORD_0);
            resources.register_shader_family(TEXT_FAMILY, std::move(family));
        }

        // The point of the tessellated text's rectangle that the style asks to
        // be put on the node's origin. Subtracting it from every vertex is what
        // bakes the anchor into the geometry, which is why anchoring costs
        // nothing per frame and needs no uniform.
        Xyz::Vector2F anchor_offset(const TextStyle& style,
                                    const Xyz::RectangleF& rect)
        {
            Xyz::Vector2F offset;
            switch (style.horizontal_anchor)
            {
            case HorizontalAnchor::LEFT:
                offset.x() = rect.origin.x();
                break;
            case HorizontalAnchor::CENTER:
                offset.x() = rect.origin.x() + rect.size.x() / 2;
                break;
            case HorizontalAnchor::RIGHT:
                offset.x() = rect.origin.x() + rect.size.x();
                break;
            }

            switch (style.vertical_anchor)
            {
            case VerticalAnchor::TOP:
                offset.y() = rect.origin.y() + rect.size.y();
                break;
            case VerticalAnchor::CENTER:
                offset.y() = rect.origin.y() + rect.size.y() / 2;
                break;
            case VerticalAnchor::BOTTOM:
                offset.y() = rect.origin.y();
                break;
            case VerticalAnchor::BASELINE:
                // The layout code puts the first line's baseline at y = 0, so
                // the origin is already the baseline.
                offset.y() = 0;
                break;
            }
            return offset;
        }

        // The anchored rectangle as bounds for the culler. Flat in z, and the
        // corners are min/maxed rather than assumed ordered, because a
        // rectangle's size is not required to be positive.
        Xyz::BBox3F make_bounds(const Xyz::RectangleF& rect,
                                const Xyz::Vector2F& offset)
        {
            const auto a = rect.origin - offset;
            const auto b = a + rect.size;
            return {
                {std::min(a.x(), b.x()), std::min(a.y(), b.y()), 0.f},
                {std::max(a.x(), b.x()), std::max(a.y(), b.y()), 0.f}
            };
        }
    }

    struct TextSystem::Members
    {
        explicit Members(ResourceManager& resources);

        void update(Scene& scene);

        [[nodiscard]]
        size_t live_item_count() const;

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

        ResourceManager& resources;
        BufferArenaRef vertex_arena;
        BufferArenaRef index_arena;
        VertexLayoutRef layout;
        ShaderProgramRef shader;

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
         * free_slots instead — so the index a component stores stays valid.
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
        std::vector<Atlas> atlases;
        std::vector<MaterialEntry> materials;

        std::vector<Entry> entries;
        std::vector<uint32_t> free_slots;
        /**
         * Which entries a live component claimed during the current sweep.
         * Whatever is left unclaimed afterwards belongs to a component that
         * was removed, or whose node was, and is released. Kept as a member so
         * the per-frame sweep does not allocate.
         */
        std::vector<uint8_t> claimed;

        /**
         * Tessellation scratch, reused across items and frames so that a
         * steady state — a handful of values changing each frame — does not
         * allocate. The glyph_* pair is what the font layout code fills (its
         * GlyphVertex is this tuple); vertexes and indexes are the anchored,
         * rebased form that goes to the GPU.
         */
        std::vector<GlyphVertex> glyph_vertexes;
        std::vector<int32_t> glyph_indexes;
        std::vector<TextVertex> vertexes;
        std::vector<uint32_t> indexes;
    };

    TextSystem::Members::Members(ResourceManager& resources)
        : resources(resources)
    {
        layout = resources.register_layout(VertexLayoutBuilder()
            .add_attribute(Tungsten::AttributeSemantic::POSITION)
            .set_component_count(2)
            .add_attribute(Tungsten::AttributeSemantic::TEX_COORD_0)
            .build());
        register_text_family(resources);
        // The family has no feature flags, so there is exactly one variant and
        // it can be resolved once here rather than per material.
        shader = resources.register_shader_variant({TEXT_FAMILY, 0});

        vertex_arena = resources.create_arena(BufferUsage::DYNAMIC_DRAW,
                                              VERTEX_STRIDE,
                                              INITIAL_VERTEX_CAPACITY);
        index_arena = resources.create_arena(BufferUsage::DYNAMIC_DRAW,
                                             INDEX_STRIDE,
                                             INITIAL_INDEX_CAPACITY);
    }

    void TextSystem::Members::update(Scene& scene)
    {
        auto& store = scene.components<TextComponent>();

        claimed.assign(entries.size(), 0);

        for (size_t i = 0; i < store.items.size(); ++i)
        {
            update_component(scene, store.owners[i], store.items[i]);

            // Claimed after the fact rather than inside update_component, so
            // that every path through it — rebuilt, unchanged, or released —
            // is covered by one statement. acquire_slot() has grown claimed
            // to match by now if a slot was taken.
            const auto slot = store.items[i].built.slot;
            if (slot != TextComponent::BuiltState::NO_SLOT)
                claimed[slot] = 1;
        }

        // Whatever no live component claimed belongs to one that has been
        // removed — or whose node was, taking the component with it. This is
        // the only place text meshes are reclaimed: Scene knows nothing about
        // the GPU memory a TextComponent stands for.
        for (uint32_t slot = 0; slot < entries.size(); ++slot)
        {
            if (!claimed[slot] && entries[slot].mesh)
                release_slot(slot);
        }
    }

    size_t TextSystem::Members::live_item_count() const
    {
        return entries.size() - free_slots.size();
    }

    void TextSystem::Members::update_component(Scene& scene, NodeId owner,
                                               TextComponent& text)
    {
        auto& built = text.built;

        // The diff that replaces dirty flags. Styles are immutable and shared,
        // so comparing the pointer is enough — and is what makes a style
        // change cheap to detect across hundreds of items.
        const bool geometry_changed = built.style != text.style
                                      || built.text != text.text;
        const bool material_changed = built.style != text.style
                                      || built.color_override
                                      != text.color_override;
        const bool renderable_changed = geometry_changed || material_changed
                                        || built.visible != text.visible
                                        || built.render_layer
                                        != text.render_layer;
        if (!renderable_changed)
            return;

        const bool has_font = text.style && text.style->font;

        if (geometry_changed)
            rebuild_geometry(text);

        if (material_changed)
        {
            built.material = has_font
                                 ? resolve_material(*text.style,
                                                    text.color_override)
                                 : MaterialRef{};
        }

        built.text = text.text;
        built.style = text.style;
        built.color_override = text.color_override;
        built.visible = text.visible;
        built.render_layer = text.render_layer;

        // Publish. A component that has never had anything to draw gets no
        // RenderableComponent at all, so empty text costs the extraction pass
        // nothing; once it has one, it is kept and simply switched off.
        auto* renderable = scene.find_component<RenderableComponent>(owner);
        if (!renderable)
        {
            if (!built.mesh)
                return;
            scene.add_component(owner, RenderableComponent{});
            // add_component invalidates references into the store it grew, so
            // the component is looked up again rather than kept from the call.
            renderable = scene.find_component<RenderableComponent>(owner);
        }

        renderable->mesh = built.mesh;
        renderable->material = built.material;
        renderable->local_bounds = built.bounds;
        renderable->visible = text.visible && bool(built.mesh);
        renderable->render_layer = text.render_layer;
    }

    void TextSystem::Members::rebuild_geometry(TextComponent& text)
    {
        auto& built = text.built;

        vertexes.clear();
        indexes.clear();
        Xyz::RectangleF rect;
        Xyz::Vector2F offset;

        if (text.style && text.style->font && !text.text.empty())
        {
            const TextStyle& style = *text.style;
            glyph_vertexes.clear();
            glyph_indexes.clear();
            rect = add_vertexes(glyph_vertexes, glyph_indexes, *style.font,
                                utf8_to_utf32(text.text), style.line_gap,
                                style.horizontal_alignment);
            offset = anchor_offset(style, rect);

            vertexes.reserve(glyph_vertexes.size());
            for (const auto& [position, tex_coord] : glyph_vertexes)
                vertexes.push_back({position - offset, tex_coord});
        }

        if (vertexes.empty())
        {
            // Nothing to draw. Releasing rather than keeping a zero-length
            // mesh means empty text holds no arena space and no mesh slot —
            // and it keeps Mesh's invariant that its slices name live arenas.
            if (built.slot != TextComponent::BuiltState::NO_SLOT)
            {
                release_slot(built.slot);
                built.slot = TextComponent::BuiltState::NO_SLOT;
            }
            built.mesh = {};
            built.bounds = {};
            return;
        }

        if (built.slot == TextComponent::BuiltState::NO_SLOT)
            built.slot = acquire_slot();

        // Free the existing mesh.
        if (built.mesh)
        {
            Mesh& previous = resources.get_mesh(built.mesh);
            resources.free(previous.streams[0]);
            resources.free(previous.ebo);
        }

        const SharedBuffer vertices = resources.allocate(
            vertex_arena, static_cast<uint32_t>(vertexes.size()));

        // The layout code numbers a text's vertices from zero; the renderer
        // draws with absolute indices, so they are rebased onto wherever the
        // arena just put this item's vertices.
        indexes.reserve(glyph_indexes.size());
        for (const int32_t index : glyph_indexes)
            indexes.push_back(uint32_t(index) + vertices.offset);

        const SharedBuffer indices = resources.allocate(
            index_arena, static_cast<uint32_t>(indexes.size()));

        resources.upload(vertices, vertexes.data(),
                         vertexes.size() * sizeof(TextVertex));
        resources.upload(indices, indexes.data(),
                         indexes.size() * sizeof(uint32_t));

        if (built.mesh)
        {
            Mesh& mesh = resources.get_mesh(built.mesh);
            mesh.streams[0] = vertices;
            mesh.ebo = indices;
        }
        else
        {
            // Created with its slices already in place, so create_mesh can
            // check the layout against the arenas it will actually be read
            // from rather than against an empty placeholder stream.
            Mesh mesh;
            mesh.layout = layout;
            mesh.streams = {vertices};
            mesh.ebo = indices;
            mesh.index_type = ElementIndexType::UINT32;
            mesh.primitive = TopologyType::TRIANGLES;
            // Every text mesh draws from the same two arenas with the same
            // layout, so they all resolve to one cached VAO. Arena growth
            // re-points it in place, which is why the id can be taken once.
            const BufferArenaRef vbo_arenas[] = {vertex_arena};
            mesh.vao = resources.get_vao(vbo_arenas, index_arena, layout);
            built.mesh = resources.create_mesh(std::move(mesh));
            entries[built.slot].mesh = built.mesh;
        }

        built.bounds = make_bounds(rect, offset);
    }

    MaterialRef TextSystem::Members::resolve_material(
        const TextStyle& style,
        const std::optional<Xyz::Vector4F>& color_override)
    {
        const TextureRef atlas = get_atlas(style.font);
        const Xyz::Vector4F color = color_override.value_or(style.color);

        for (const auto& entry : materials)
        {
            if (entry.atlas == atlas && entry.color == color)
                return entry.material;
        }

        Material material;
        material.shader = shader;
        // The MaterialBlock is one vec4. It is written as an opaque blob here,
        // exactly as the shader declares it; ResourceManager uploads it into
        // the material's own UBO without interpreting it.
        material.parameter_data.resize(sizeof(float) * 4);
        std::memcpy(material.parameter_data.data(), color.values.data(),
                    sizeof(float) * 4);
        material.textures = {atlas};
        // Glyph edges are antialiased, so text always blends: it belongs in
        // the back-to-front pass, never the opaque one.
        material.transparent = true;

        const auto ref = resources.create_material(std::move(material));
        materials.push_back({atlas, color, ref});
        return ref;
    }

    TextureRef TextSystem::Members::get_atlas(
        const std::shared_ptr<const Font>& font)
    {
        for (const auto& atlas : atlases)
        {
            if (atlas.font == font)
                return atlas.texture;
        }

        const Yimage::Image& image = font->image;
        const auto source_format = get_ogl_pixel_type(image.pixel_type());

        Texture texture;
        texture.gl_handle = generate_texture();
        bind_texture(TextureTarget::TEXTURE_2D, texture.gl_handle.id());
        set_texture_image_2d(TextureTarget2D::TEXTURE_2D, 0, get_size(image),
                             source_format, image.data());
        texture.width = static_cast<uint32_t>(image.width());
        texture.height = static_cast<uint32_t>(image.height());
        texture.format = source_format.format;
        // A null sampler ref resolves to the manager's default — linear, no
        // mipmaps, clamped — which is exactly what an atlas wants, so there is
        // no descriptor to register.

        const auto ref = resources.create_texture(std::move(texture));
        atlases.push_back({font, ref});
        return ref;
    }

    uint32_t TextSystem::Members::acquire_slot()
    {
        if (!free_slots.empty())
        {
            const auto slot = free_slots.back();
            free_slots.pop_back();
            return slot;
        }
        entries.emplace_back();
        // Kept the same length as entries, so the sweep at the end of update()
        // can index it with any slot handed out during that same sweep.
        claimed.push_back(0);
        return static_cast<uint32_t>(entries.size() - 1);
    }

    void TextSystem::Members::release_slot(uint32_t slot)
    {
        if (slot >= entries.size())
            TUNGSTEN_THROW("TextSystem: invalid slot.");

        Entry& entry = entries[slot];
        if (entry.mesh)
        {
            // Destroying the mesh is what returns its vertex and index slices
            // to the arenas; the mesh owns no GL object of its own.
            resources.destroy_mesh(entry.mesh);
            entry.mesh = {};
        }
        free_slots.push_back(slot);
    }

    TextSystem::TextSystem(ResourceManager& resources)
        : members_(std::make_unique<Members>(resources))
    {
    }

    TextSystem::~TextSystem() = default;

    void TextSystem::update(Scene& scene)
    {
        members_->update(scene);
    }

    size_t TextSystem::live_item_count() const
    {
        return members_->live_item_count();
    }
} // Tungsten
