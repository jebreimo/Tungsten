//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/Renderer.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Neo/CameraComponent.hpp"
#include "Tungsten/Neo/NodeHandle.hpp"
#include "Tungsten/Neo/RenderableComponent.hpp"
#include "Tungsten/Neo/ResourceManager.hpp"
#include "Tungsten/Neo/Scene.hpp"
#include "Tungsten/Neo/SnapshotBuilder.hpp"
#include "Tungsten/Neo/TransformUpdater.hpp"
#include "Tungsten/TungstenException.hpp"
#include "../../src/Tungsten/Neo/UboBindings.hpp"

using namespace Tungsten;

namespace
{
    // Fabricates GL ids and records the calls the renderer's behavior is
    // specified by: draws (with their index offsets, to observe ordering),
    // program binds, buffer uploads (by size, to tell the three UBOs apart),
    // uniform-block bindings, and blend toggles.
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

        GLuint create_shader(GLenum) override
        {
            return next_id_++;
        }

        GLuint create_program() override
        {
            return next_id_++;
        }

        // ShaderPreprocessor derives its #version from these version strings.
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
            // Fabricated indices, distinct per known block name.
            if (std::strcmp(name, "PerFrame") == 0)
                return 10;
            if (std::strcmp(name, "MaterialBlock") == 0)
                return 11;
            if (std::strcmp(name, "PerDraw") == 0)
                return 12;
            return 0xFFFFFFFFu;
        }

        void uniform_block_binding(GLuint, GLuint block_index,
                                   GLuint binding) override
        {
            block_bindings.emplace_back(block_index, binding);
        }

        void use_program(GLuint program) override
        {
            ++program_binds;
        }

        // Fabricated locations, distinct per known sampler name, so the
        // tests can tell which sampler was pointed at which unit.
        GLint get_uniform_location(GLuint, const GLchar* name) override
        {
            if (std::strcmp(name, "u_diffuse_map") == 0)
                return 20;
            if (std::strcmp(name, "u_specular_map") == 0)
                return 21;
            return -1;
        }

        void uniform1_i(GLint location, GLint v0) override
        {
            int_uniforms.emplace_back(location, v0);
        }

        void active_texture(GLenum texture) override
        {
            current_texture_unit_ = static_cast<int>(texture - 0x84C0);
        }

        void bind_texture(GLenum, GLuint texture) override
        {
            texture_binds.emplace_back(current_texture_unit_, texture);
        }

        void buffer_data(GLenum, GLsizeiptr size, const void*, GLenum) override
        {
            buffer_uploads.push_back(static_cast<size_t>(size));
        }

        void draw_elements(GLenum, GLsizei count, GLenum,
                           const void* indices) override
        {
            events.push_back("draw@" + std::to_string(
                reinterpret_cast<intptr_t>(indices)));
            ++draw_calls;
            last_draw_count = count;
        }

        void draw_arrays(GLenum, GLint first, GLsizei) override
        {
            events.push_back("draw_arrays@" + std::to_string(first));
            ++draw_calls;
        }

        void enable(GLenum cap) override
        {
            if (cap == 0x0BE2) // GL_BLEND
                events.emplace_back("blend_on");
        }

        void disable(GLenum cap) override
        {
            if (cap == 0x0BE2) // GL_BLEND
                events.emplace_back("blend_off");
        }

        std::vector<std::pair<GLuint, GLuint>> block_bindings;
        std::vector<std::pair<GLint, GLint>> int_uniforms;
        std::vector<std::pair<int, GLuint>> texture_binds;
        std::vector<size_t> buffer_uploads;
        std::vector<std::string> events;
        int program_binds = 0;
        int draw_calls = 0;
        int last_draw_count = 0;

    private:
        GLuint next_id_ = 1;
        int current_texture_unit_ = 0;
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

    constexpr size_t PER_FRAME_SIZE = 176 + MAX_LIGHTS * 64;
    constexpr size_t PER_DRAW_SIZE = RENDER_ITEM_DATA_SIZE * sizeof(float);
    constexpr size_t MATERIAL_BLOB_SIZE = 64;

    size_t count_uploads(const std::vector<size_t>& uploads, size_t size)
    {
        size_t n = 0;
        for (const auto upload : uploads)
        {
            if (upload == size)
                ++n;
        }
        return n;
    }

    // The full pipeline in one place: resources (arena-backed meshes, a
    // compiled shader variant, materials), a scene of renderable nodes, and
    // the snapshot built from it.
    struct Bench
    {
        Bench()
        {
            VertexLayout layout_value;
            layout_value.attributes.push_back(
                {AttributeSemantic::POSITION, 0,
                 VertexAttributeDataType::FLOAT, 3, false, 0});
            layout_value.stride = 12;
            layout = resources.register_layout(layout_value);

            vbo_arena = resources.create_arena(BufferUsage::STATIC_DRAW,
                                               12, 256);
            ebo_arena = resources.create_arena(BufferUsage::STATIC_DRAW,
                                               2, 256);

            ShaderFamily family;
            family.vertex_source = "#version 300 es\nvoid main() {}\n";
            family.fragment_source = "#version 300 es\nvoid main() {}\n";
            family.required_layout = layout;
            resources.register_shader_family(1, family);
            shader = resources.register_shader_variant({1, 0});

            Material material_value;
            material_value.shader = shader;
            material_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
            material = resources.create_material(std::move(material_value));

            camera_node = scene.add_node();
            camera_node.add(CameraComponent{
                .mode = ProjectionMode::ORTHOGRAPHIC,
                .far_plane = 1000.0f,
                .ortho_size = 100.0f
            });
        }

        MeshRef make_mesh(uint32_t vertex_count, uint32_t index_count)
        {
            Mesh mesh;
            const BufferArenaRef vbos[] = {vbo_arena};
            mesh.vao = resources.get_vao(vbos, ebo_arena, layout);
            mesh.streams = {resources.allocate(vbo_arena, vertex_count)};
            mesh.layout = layout;
            mesh.ebo = resources.allocate(ebo_arena, index_count);
            return resources.create_mesh(std::move(mesh));
        }

        NodeHandle add_renderable(MeshRef mesh, MaterialRef mat, float z)
        {
            auto node = scene.add_node();
            Transform transform;
            transform.translation = {0, 0, z};
            node.set_local_transform(transform);
            node.add(RenderableComponent{.mesh = mesh, .material = mat});
            return node;
        }

        void build_and_render(FakeOglWrapper& gl)
        {
            TransformUpdater::resolve(scene);
            SnapshotBuilder(resources)
                .build(scene, camera_node.id(), snapshot);
            Renderer renderer(resources);
            // The constructor allocates the UBOs through buffer_data and
            // creates the white texture; reset the recording so the tests
            // observe only render().
            gl.buffer_uploads.clear();
            gl.events.clear();
            gl.texture_binds.clear();
            renderer.render(snapshot);
        }

        ResourceManager resources;
        Scene scene;
        NodeHandle camera_node;
        RenderSnapshot snapshot;
        VertexLayoutRef layout;
        BufferArenaRef vbo_arena;
        BufferArenaRef ebo_arena;
        ShaderProgramRef shader;
        MaterialRef material;
    };
}

TEST_CASE("ShaderLibrary: variants get the fixed UBO binding points")
{
    const FakeGlSession session;
    Bench bench;

    using Pair = std::pair<GLuint, GLuint>;
    const auto& bindings = session.gl->block_bindings;
    REQUIRE(bindings.size() == 3);
    REQUIRE(bindings[0] == Pair{10, PER_FRAME_UBO_BINDING});
    REQUIRE(bindings[1] == Pair{11, PER_MATERIAL_UBO_BINDING});
    REQUIRE(bindings[2] == Pair{12, PER_DRAW_UBO_BINDING});
}

TEST_CASE("ShaderLibrary: sampler uniforms get consecutive texture units")
{
    const FakeGlSession session;
    ResourceManager resources;

    ShaderFamily family;
    family.vertex_source = "#version 300 es\nvoid main() {}\n";
    family.fragment_source = "#version 300 es\nvoid main() {}\n";
    family.samplers = {"u_diffuse_map", "u_specular_map"};
    resources.register_shader_family(1, family);
    resources.register_shader_variant({1, 0});

    using Pair = std::pair<GLint, GLint>;
    REQUIRE(session.gl->int_uniforms == std::vector<Pair>{{20, 0}, {21, 1}});
}

TEST_CASE("Renderer: unfilled sampler units get the white texture")
{
    FakeGlSession session;
    Bench bench;

    // A family with two samplers, drawn with a material that binds no
    // textures: both units must be filled with the renderer's white texture.
    ShaderFamily family;
    family.vertex_source = "#version 300 es\nvoid main() {}\n";
    family.fragment_source = "#version 300 es\nvoid main() {}\n";
    family.samplers = {"u_diffuse_map", "u_specular_map"};
    family.required_layout = bench.layout;
    bench.resources.register_shader_family(2, family);

    Material material_value;
    material_value.shader = bench.resources.register_shader_variant({2, 0});
    // The fake reports every conventional block as present, so a material for
    // it has to carry its blob — as it would for any shader that reads one.
    material_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
    const auto material = bench.resources.create_material(
        std::move(material_value));
    bench.add_renderable(bench.make_mesh(4, 6), material, -10);

    bench.build_and_render(*session.gl);
    const auto& binds = session.gl->texture_binds;
    REQUIRE(binds.size() == 2);
    REQUIRE(binds[0].first == 0);
    REQUIRE(binds[1].first == 1);
    // Both units hold the same texture — the constructor's 1×1 white.
    REQUIRE(binds[0].second != 0);
    REQUIRE(binds[0].second == binds[1].second);
}

TEST_CASE("Renderer: a material with no parameters for a shader that reads"
          " them is rejected")
{
    const FakeGlSession session;
    Bench bench;

    // parameter_data left empty. Drawing it would show whatever the previous
    // material uploaded, or an empty UBO on the first draw of the frame.
    Material bare;
    bare.shader = bench.shader;
    const auto material = bench.resources.create_material(std::move(bare));
    bench.add_renderable(bench.make_mesh(4, 6), material, -10);

    REQUIRE_THROWS_AS(bench.build_and_render(*session.gl), TungstenException);
}

TEST_CASE("Renderer: draws every item with one draw call each")
{
    const FakeGlSession session;
    Bench bench;
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -20);

    bench.build_and_render(*session.gl);
    REQUIRE(session.gl->draw_calls == 2);
    REQUIRE(session.gl->last_draw_count == 6);
}

TEST_CASE("Renderer: uploads the per-frame block once, the per-draw block per item")
{
    const FakeGlSession session;
    Bench bench;
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -20);

    bench.build_and_render(*session.gl);
    REQUIRE(count_uploads(session.gl->buffer_uploads, PER_FRAME_SIZE) == 1);
    REQUIRE(count_uploads(session.gl->buffer_uploads, PER_DRAW_SIZE) == 2);
}

TEST_CASE("Renderer: binds program and material once for a batched run")
{
    const FakeGlSession session;
    Bench bench;
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -20);
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -30);

    bench.build_and_render(*session.gl);
    // All three items share one material: one glUseProgram, one blob upload.
    REQUIRE(session.gl->program_binds == 1);
    REQUIRE(count_uploads(session.gl->buffer_uploads, MATERIAL_BLOB_SIZE) == 1);
}

TEST_CASE("Renderer: re-binds the material when it changes between items")
{
    const FakeGlSession session;
    Bench bench;
    Material second_value;
    second_value.shader = bench.shader;
    second_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
    const auto second = bench.resources.create_material(
        std::move(second_value));

    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, bench.material, -10);
    bench.add_renderable(mesh, second, -20);
    bench.add_renderable(mesh, bench.material, -30);

    bench.build_and_render(*session.gl);
    // The sort groups the two bench.material items together, so three items
    // over two materials cost two material binds, not three.
    REQUIRE(count_uploads(session.gl->buffer_uploads, MATERIAL_BLOB_SIZE) == 2);
}

TEST_CASE("Renderer: draws items in sort-key order, not insertion order")
{
    const FakeGlSession session;
    Bench bench;
    // Two meshes: their ebo slices get different offsets, and the sort key
    // orders opaque items by mesh index, so the draws must come out in mesh
    // order even though the scene lists them reversed.
    const auto mesh_a = bench.make_mesh(4, 6);  // ebo offset 0
    const auto mesh_b = bench.make_mesh(4, 6);  // ebo offset 8 (buddy rounds)
    bench.add_renderable(mesh_b, bench.material, -10);
    bench.add_renderable(mesh_a, bench.material, -20);

    bench.build_and_render(*session.gl);
    std::vector<std::string> draws;
    for (const auto& event : session.gl->events)
    {
        if (event.starts_with("draw@"))
            draws.push_back(event);
    }
    REQUIRE(draws.size() == 2);
    // mesh_a's slice starts at byte 0, mesh_b's after it: ascending order.
    REQUIRE(draws[0] < draws[1]);
}

TEST_CASE("Renderer: transparent items draw after opaque, with blending")
{
    const FakeGlSession session;
    Bench bench;
    Material transparent_value;
    transparent_value.shader = bench.shader;
    transparent_value.transparent = true;
    transparent_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
    const auto transparent = bench.resources.create_material(
        std::move(transparent_value));

    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, transparent, -10);
    bench.add_renderable(mesh, bench.material, -20);

    bench.build_and_render(*session.gl);
    const auto& events = session.gl->events;
    // Expect: opaque draw, blend on, transparent draw, blend off. (The
    // constructor and per-frame setup emit no draw or blend events, and the
    // initial set_blend_enabled(false) is the leading blend_off.)
    std::vector<std::string> relevant;
    for (const auto& event : events)
    {
        if (event.starts_with("draw") || event.starts_with("blend"))
            relevant.push_back(event.starts_with("draw") ? "draw" : event);
    }
    REQUIRE(relevant == std::vector<std::string>{
        "blend_off", "draw", "blend_on", "draw", "blend_off"});
}

TEST_CASE("Renderer: a mesh without an index buffer uses an array draw")
{
    const FakeGlSession session;
    Bench bench;
    Mesh mesh_value;
    const BufferArenaRef vbos[] = {bench.vbo_arena};
    mesh_value.vao = bench.resources.get_vao(vbos, bench.ebo_arena,
                                             bench.layout);
    mesh_value.streams = {bench.resources.allocate(bench.vbo_arena, 8)};
    mesh_value.layout = bench.layout;
    // ebo left null: not an indexed mesh.
    const auto mesh = bench.resources.create_mesh(std::move(mesh_value));
    bench.add_renderable(mesh, bench.material, -10);

    bench.build_and_render(*session.gl);
    REQUIRE(session.gl->draw_calls == 1);
    REQUIRE(session.gl->events.back().starts_with("draw_arrays@"));
}
