//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Rendering/TextSystem.hpp"

#include <cstring>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/SceneGraph/NodeHandle.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"
#include "Tungsten/SceneGraph/TextComponent.hpp"
#include "Tungsten/Rendering/TextStyle.hpp"
#include "Tungsten/Rendering/FontManager.hpp"

using namespace Tungsten;

namespace
{
    // Fabricates GL ids and records the one thing these tests are about:
    // which sub-ranges of which buffers were written, so a rebuild can be told
    // from a no-op without inspecting the GPU.
    class FakeOglWrapper : public DummyOglWrapper
    {
    public:
        void gen_buffers(GLsizei n, GLuint* buffers) override
        {
            for (GLsizei i = 0; i < n; ++i)
                buffers[i] = next_id_++;
        }

        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
            for (GLsizei i = 0; i < n; ++i)
                arrays[i] = next_id_++;
        }

        void gen_textures(GLsizei n, GLuint* textures) override
        {
            for (GLsizei i = 0; i < n; ++i)
                textures[i] = next_id_++;
        }

        void gen_samplers(GLsizei n, GLuint* samplers) override
        {
            for (GLsizei i = 0; i < n; ++i)
                samplers[i] = next_id_++;
        }

        GLuint create_shader(GLenum) override
        {
            return next_id_++;
        }

        GLuint create_program() override
        {
            return next_id_++;
        }

        // ShaderPreprocessor derives its #version from these.
        const GLubyte* get_string(GLenum name) override
        {
            if (name == 0x8B8C) // GL_SHADING_LANGUAGE_VERSION
                return reinterpret_cast<const GLubyte*>("3.00");
            if (name == 0x1F02) // GL_VERSION
                return reinterpret_cast<const GLubyte*>("OpenGL ES 3.0");
            return nullptr;
        }

        void get_shader(GLuint, GLenum, GLint* param) override
        {
            *param = 1; // compiled
        }

        void get_program(GLuint, GLenum, GLint* param) override
        {
            *param = 1; // linked
        }

        GLuint get_uniform_block_index(GLuint, const GLchar* name) override
        {
            if (std::strcmp(name, "PerFrame") == 0)
                return 10;
            if (std::strcmp(name, "MaterialBlock") == 0)
                return 11;
            if (std::strcmp(name, "PerDraw") == 0)
                return 12;
            return 0xFFFFFFFFu;
        }

        GLint get_uniform_location(GLuint, const GLchar* name) override
        {
            return std::strcmp(name, "u_text_atlas") == 0 ? 20 : -1;
        }

        void buffer_sub_data(GLenum, GLintptr offset, GLsizeiptr size,
                             const void*) override
        {
            sub_uploads.push_back({static_cast<size_t>(offset),
                                   static_cast<size_t>(size)});
        }

        struct Upload
        {
            size_t offset;
            size_t size;
        };

        std::vector<Upload> sub_uploads;

    private:
        GLuint next_id_ = 1;
    };

    struct FakeGlSession
    {
        FakeGlSession()
        {
            auto wrapper = std::make_unique<FakeOglWrapper>();
            gl = wrapper.get();
            previous_ = set_custom_ogl_wrapper(std::move(wrapper));
        }

        ~FakeGlSession()
        {
            set_custom_ogl_wrapper(std::move(previous_));
        }

        FakeOglWrapper* gl;

    private:
        std::unique_ptr<IOglWrapper> previous_;
    };

    // The builtin Monaco 32 font, loaded once for the whole test file: it
    // carries a full atlas image, and nothing here mutates it.
    std::shared_ptr<const Font> test_font()
    {
        static const FontManager manager;
        return manager.default_font();
    }

    std::shared_ptr<const TextStyle> test_style(Xyz::Vector4F color
                                                    = {0.f, 0.f, 0.f, 1.f})
    {
        return make_text_style({.font = test_font(), .color = color});
    }
}

TEST_CASE("TextSystem: a text component is published as a renderable")
{
    FakeGlSession session;
    ResourceManager resources;
    TextSystem text_system(resources);
    Scene scene;

    auto node = scene.add_node();
    node.add(TextComponent{.text = "Total", .style = test_style()});

    // Nothing is compiled until the system runs.
    REQUIRE(node.find<RenderableComponent>() == nullptr);

    text_system.update(scene);

    const auto* renderable = node.find<RenderableComponent>();
    REQUIRE(renderable != nullptr);
    REQUIRE(bool(renderable->mesh));
    REQUIRE(bool(renderable->material));
    REQUIRE(renderable->visible);
    // Bounds are set, so text takes part in frustum culling.
    REQUIRE(bool(renderable->local_bounds));

    // Text always blends, so it belongs in the transparent pass.
    REQUIRE(resources.get_material(renderable->material).transparent);

    // Five glyphs: 4 vertices and 6 indices each.
    const Mesh& mesh = resources.get_mesh(renderable->mesh);
    REQUIRE(mesh.streams.size() == 1);
    REQUIRE(mesh.streams[0].count == 5 * 4);
    REQUIRE(mesh.ebo.count == 5 * 6);
    REQUIRE(mesh.index_type == ElementIndexType::UINT32);
}

TEST_CASE("TextSystem: only changed text is re-uploaded")
{
    FakeGlSession session;
    ResourceManager resources;
    TextSystem text_system(resources);
    Scene scene;

    const auto style = test_style();
    auto label = scene.add_node();
    label.add(TextComponent{.text = "Speed", .style = style});
    auto value = scene.add_node();
    value.add(TextComponent{.text = "10", .style = style});

    text_system.update(scene);

    SECTION("an unchanged scene uploads nothing")
    {
        session.gl->sub_uploads.clear();
        text_system.update(scene);
        REQUIRE(session.gl->sub_uploads.empty());
    }

    SECTION("moving a node uploads nothing")
    {
        session.gl->sub_uploads.clear();
        value.set_local_transform({.translation = {0.f, 20.f, 0.f}});
        text_system.update(scene);
        // Position lives in the node's transform and reaches the shader
        // through the per-draw UBO, so it never touches the vertex data.
        REQUIRE(session.gl->sub_uploads.empty());
    }

    SECTION("changing one value re-uploads only that item")
    {
        session.gl->sub_uploads.clear();
        value.get<TextComponent>().text = "11";
        text_system.update(scene);

        // Exactly one vertex upload and one index upload, for the two glyphs
        // of the changed item alone. The label is untouched.
        REQUIRE(session.gl->sub_uploads.size() == 2);
        REQUIRE(session.gl->sub_uploads[0].size == 2 * 4 * sizeof(TextVertex));
        REQUIRE(session.gl->sub_uploads[1].size == 2 * 6 * sizeof(uint32_t));
    }

    SECTION("recolouring one item uploads nothing")
    {
        const auto before = value.get<TextComponent>().built.material();
        session.gl->sub_uploads.clear();

        value.get<TextComponent>().color_override = Xyz::Vector4F{1, 0, 0, 1};
        text_system.update(scene);

        REQUIRE(session.gl->sub_uploads.empty());
        // A different material, resolved from the (atlas, colour) cache.
        REQUIRE(value.get<RenderableComponent>().material != before);
    }
}

TEST_CASE("TextSystem: items sharing a style share one material")
{
    FakeGlSession session;
    ResourceManager resources;
    TextSystem text_system(resources);
    Scene scene;

    const auto style = test_style();
    auto a = scene.add_node();
    a.add(TextComponent{.text = "1.0", .style = style});
    auto b = scene.add_node();
    b.add(TextComponent{.text = "2.0", .style = style});
    // A different style object with the same colour still resolves to the
    // same material: the cache is keyed on the atlas and the colour, not on
    // the style's identity.
    auto c = scene.add_node();
    c.add(TextComponent{.text = "3.0", .style = test_style()});
    // A different colour does not.
    auto d = scene.add_node();
    d.add(TextComponent{.text = "4.0", .style = test_style({1, 0, 0, 1})});

    text_system.update(scene);

    const auto material = a.get<RenderableComponent>().material;
    REQUIRE(b.get<RenderableComponent>().material == material);
    REQUIRE(c.get<RenderableComponent>().material == material);
    REQUIRE(d.get<RenderableComponent>().material != material);

    // Every text mesh sub-allocates from the same two arenas with the same
    // layout, so they all share one VAO.
    const auto vao = resources.get_mesh(a.get<RenderableComponent>().mesh).vao;
    REQUIRE(resources.get_mesh(d.get<RenderableComponent>().mesh).vao == vao);
}

TEST_CASE("TextSystem: empty text holds no GPU memory")
{
    FakeGlSession session;
    ResourceManager resources;
    TextSystem text_system(resources);
    Scene scene;

    auto node = scene.add_node();
    node.add(TextComponent{.text = "value", .style = test_style()});
    text_system.update(scene);
    REQUIRE(text_system.live_item_count() == 1);

    node.get<TextComponent>().text.clear();
    text_system.update(scene);

    REQUIRE(text_system.live_item_count() == 0);
    REQUIRE(!bool(node.get<RenderableComponent>().mesh));
    REQUIRE(!node.get<RenderableComponent>().visible);

    // And it comes back.
    node.get<TextComponent>().text = "value";
    text_system.update(scene);
    REQUIRE(text_system.live_item_count() == 1);
    REQUIRE(node.get<RenderableComponent>().visible);
}

TEST_CASE("TextSystem: a removed text item's mesh is reclaimed")
{
    FakeGlSession session;
    ResourceManager resources;
    TextSystem text_system(resources);
    Scene scene;

    auto keep = scene.add_node();
    keep.add(TextComponent{.text = "keep", .style = test_style()});
    auto doomed = scene.add_node();
    doomed.add(TextComponent{.text = "doomed", .style = test_style()});

    text_system.update(scene);
    REQUIRE(text_system.live_item_count() == 2);
    const auto doomed_mesh = doomed.get<TextComponent>().built.mesh();

    SECTION("when its node is destroyed")
    {
        doomed.remove();
        text_system.update(scene);

        REQUIRE(text_system.live_item_count() == 1);
        // The ref is revoked, so a stale one fails validation rather than
        // resolving to whatever took the slot.
        REQUIRE_THROWS(resources.get_mesh(doomed_mesh));
    }

    SECTION("when only the component is removed")
    {
        doomed.remove_component<TextComponent>();
        text_system.update(scene);

        REQUIRE(text_system.live_item_count() == 1);
        REQUIRE_THROWS(resources.get_mesh(doomed_mesh));
    }

    SECTION("and the freed slot is reused by the next item")
    {
        doomed.remove();
        text_system.update(scene);

        auto fresh = scene.add_node();
        fresh.add(TextComponent{.text = "fresh", .style = test_style()});
        text_system.update(scene);

        REQUIRE(text_system.live_item_count() == 2);
        REQUIRE(bool(fresh.get<RenderableComponent>().mesh));
    }
}

TEST_CASE("TextSystem: a component with no style draws nothing")
{
    FakeGlSession session;
    ResourceManager resources;
    TextSystem text_system(resources);
    Scene scene;

    auto node = scene.add_node();
    node.add(TextComponent{.text = "orphan"});

    text_system.update(scene);

    REQUIRE(text_system.live_item_count() == 0);
    // Nothing was ever drawable, so no renderable was attached at all.
    REQUIRE(node.find<RenderableComponent>() == nullptr);

    // Giving it a style is enough to bring it to life.
    node.get<TextComponent>().style = test_style();
    text_system.update(scene);
    REQUIRE(bool(node.get<RenderableComponent>().mesh));
}
