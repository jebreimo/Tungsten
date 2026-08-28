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
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Neo/BuiltinShaders.hpp"
#include "Tungsten/Neo/Material.hpp"
#include "Tungsten/Neo/Mesh.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/Scene.hpp"
#include "Tungsten/Neo/TextComponent.hpp"
#include "Tungsten/Neo/TextStyle.hpp"
#include "Tungsten/Neo/Texture.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/YimageGl.hpp"
#include "../Render/FontUtilities.hpp"
#include "../Render/TextUtilities.hpp"
#include "Shaders/BuiltinShaderSources.hpp"
#include "Tungsten/Neo/VertexLayoutBuilder.hpp"

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

    TextSystem::TextSystem(ResourceManager& resources)
        : resources_(resources)
    {
        layout_ = resources_.register_layout(VertexLayoutBuilder()
            .add_attribute(Tungsten::AttributeSemantic::POSITION)
            .set_component_count(2)
            .add_attribute(Tungsten::AttributeSemantic::TEX_COORD_0)
            .build());
        register_text_family(resources_);
        // The family has no feature flags, so there is exactly one variant and
        // it can be resolved once here rather than per material.
        shader_ = resources_.register_shader_variant({TEXT_FAMILY, 0});

        vertex_arena_ = resources_.create_arena(BufferUsage::DYNAMIC_DRAW,
                                                VERTEX_STRIDE,
                                                INITIAL_VERTEX_CAPACITY);
        index_arena_ = resources_.create_arena(BufferUsage::DYNAMIC_DRAW,
                                               INDEX_STRIDE,
                                               INITIAL_INDEX_CAPACITY);
    }

    void TextSystem::update(Scene& scene)
    {
        auto& store = scene.components<TextComponent>();

        claimed_.assign(entries_.size(), 0);

        for (size_t i = 0; i < store.items.size(); ++i)
        {
            update_component(scene, store.owners[i], store.items[i]);

            // Claimed after the fact rather than inside update_component, so
            // that every path through it — rebuilt, unchanged, or released —
            // is covered by one statement. acquire_slot() has grown claimed_
            // to match by now if a slot was taken.
            const auto slot = store.items[i].built.slot;
            if (slot != TextComponent::BuiltState::NO_SLOT)
                claimed_[slot] = 1;
        }

        // Whatever no live component claimed belongs to one that has been
        // removed — or whose node was, taking the component with it. This is
        // the only place text meshes are reclaimed: Scene knows nothing about
        // the GPU memory a TextComponent stands for.
        for (uint32_t slot = 0; slot < entries_.size(); ++slot)
        {
            if (!claimed_[slot] && entries_[slot].mesh)
                release_slot(slot);
        }
    }

    size_t TextSystem::live_item_count() const
    {
        return entries_.size() - free_slots_.size();
    }

    void TextSystem::update_component(Scene& scene, NodeId owner,
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

    void TextSystem::rebuild_geometry(TextComponent& text)
    {
        auto& built = text.built;

        vertexes_.clear();
        indexes_.clear();
        Xyz::RectangleF rect;
        Xyz::Vector2F offset;

        if (text.style && text.style->font && !text.text.empty())
        {
            const TextStyle& style = *text.style;
            glyph_vertexes_.clear();
            glyph_indexes_.clear();
            rect = add_vertexes(glyph_vertexes_, glyph_indexes_, *style.font,
                                utf8_to_utf32(text.text), style.line_gap,
                                style.horizontal_alignment);
            offset = anchor_offset(style, rect);

            vertexes_.reserve(glyph_vertexes_.size());
            for (const auto& [position, tex_coord] : glyph_vertexes_)
                vertexes_.push_back({position - offset, tex_coord});
        }

        if (vertexes_.empty())
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
            Mesh& previous = resources_.get_mesh(built.mesh);
            resources_.free(previous.streams[0]);
            resources_.free(previous.ebo);
        }

        const SharedBuffer vertices = resources_.allocate(
            vertex_arena_, static_cast<uint32_t>(vertexes_.size()));

        // The layout code numbers a text's vertices from zero; the renderer
        // draws with absolute indices, so they are rebased onto wherever the
        // arena just put this item's vertices.
        indexes_.reserve(glyph_indexes_.size());
        for (const int32_t index : glyph_indexes_)
            indexes_.push_back(uint32_t(index) + vertices.offset);

        const SharedBuffer indices = resources_.allocate(
            index_arena_, static_cast<uint32_t>(indexes_.size()));

        resources_.upload(vertices, vertexes_.data(),
                          vertexes_.size() * sizeof(TextVertex));
        resources_.upload(indices, indexes_.data(),
                          indexes_.size() * sizeof(uint32_t));

        if (built.mesh)
        {
            Mesh& mesh = resources_.get_mesh(built.mesh);
            mesh.streams[0] = vertices;
            mesh.ebo = indices;
        }
        else
        {
            // Created with its slices already in place, so create_mesh can
            // check the layout against the arenas it will actually be read
            // from rather than against an empty placeholder stream.
            Mesh mesh;
            mesh.layout = layout_;
            mesh.streams = {vertices};
            mesh.ebo = indices;
            mesh.index_type = ElementIndexType::UINT32;
            mesh.primitive = TopologyType::TRIANGLES;
            // Every text mesh draws from the same two arenas with the same
            // layout, so they all resolve to one cached VAO. Arena growth
            // re-points it in place, which is why the id can be taken once.
            const BufferArenaRef vbo_arenas[] = {vertex_arena_};
            mesh.vao = resources_.get_vao(vbo_arenas, index_arena_, layout_);
            built.mesh = resources_.create_mesh(std::move(mesh));
            entries_[built.slot].mesh = built.mesh;
        }

        built.bounds = make_bounds(rect, offset);
    }

    MaterialRef TextSystem::resolve_material(
        const TextStyle& style,
        const std::optional<Xyz::Vector4F>& color_override)
    {
        const TextureRef atlas = get_atlas(style.font);
        const Xyz::Vector4F color = color_override.value_or(style.color);

        for (const auto& entry : materials_)
        {
            if (entry.atlas == atlas && entry.color == color)
                return entry.material;
        }

        Material material;
        material.shader = shader_;
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

        const auto ref = resources_.create_material(std::move(material));
        materials_.push_back({atlas, color, ref});
        return ref;
    }

    TextureRef TextSystem::get_atlas(const std::shared_ptr<const Font>& font)
    {
        for (const auto& atlas : atlases_)
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

        const auto ref = resources_.create_texture(std::move(texture));
        atlases_.push_back({font, ref});
        return ref;
    }

    uint32_t TextSystem::acquire_slot()
    {
        if (!free_slots_.empty())
        {
            const auto slot = free_slots_.back();
            free_slots_.pop_back();
            return slot;
        }
        entries_.emplace_back();
        // Kept the same length as entries_, so the sweep at the end of update()
        // can index it with any slot handed out during that same sweep.
        claimed_.push_back(0);
        return static_cast<uint32_t>(entries_.size() - 1);
    }

    void TextSystem::release_slot(uint32_t slot)
    {
        if (slot >= entries_.size())
            TUNGSTEN_THROW("TextSystem: invalid slot.");

        Entry& entry = entries_[slot];
        if (entry.mesh)
        {
            // Destroying the mesh is what returns its vertex and index slices
            // to the arenas; the mesh owns no GL object of its own.
            resources_.destroy_mesh(entry.mesh);
            entry.mesh = {};
        }
        free_slots_.push_back(slot);
    }
} // Tungsten
