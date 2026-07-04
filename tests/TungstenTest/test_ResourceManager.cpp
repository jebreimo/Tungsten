//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/ResourceManager.hpp"

#include <array>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Gl/GlTexture.hpp"

using namespace Tungsten;

namespace
{
    // Fabricates GL ids and counts live objects, so the tests can observe
    // creation, deferred deletion and reuse without a GL context.
    class FakeOglWrapper : public DummyOglWrapper
    {
    public:
        void gen_buffers(GLsizei n, GLuint* buffers) override
        {
            for (GLsizei i = 0; i < n; ++i)
                buffers[i] = next_id_++;
            live_buffers += n;
        }

        void delete_buffers(GLsizei n, const GLuint*) override
        {
            live_buffers -= n;
        }

        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
            for (GLsizei i = 0; i < n; ++i)
                arrays[i] = next_id_++;
            live_vertex_arrays += n;
        }

        void delete_vertex_arrays(GLsizei n, const GLuint*) override
        {
            live_vertex_arrays -= n;
        }

        void gen_textures(GLsizei n, GLuint* textures) override
        {
            for (GLsizei i = 0; i < n; ++i)
                textures[i] = next_id_++;
            live_textures += n;
        }

        void delete_textures(GLsizei n, const GLuint*) override
        {
            live_textures -= n;
        }

        GLuint create_shader(GLenum) override
        {
            return next_id_++;
        }

        GLuint create_program() override
        {
            ++live_programs;
            return next_id_++;
        }

        void delete_program(GLuint) override
        {
            --live_programs;
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

        // Report success for every status query (compile and link status),
        // so ShaderLibrary's compile path runs without a real compiler.
        void get_shader(GLuint, GLenum, GLint* param) override
        {
            *param = 1;
        }

        void get_program(GLuint, GLenum, GLint* param) override
        {
            *param = 1;
        }

        // clone_buffer (the grow path) queries the source buffer's usage and
        // size; answer with a valid usage and an empty size.
        void get_buffer_parameter(GLenum, GLenum pname, GLint* params) override
        {
            constexpr GLenum GL_BUFFER_USAGE_ = 0x8765;
            constexpr GLint GL_STATIC_DRAW_ = 0x88E4;
            *params = pname == GL_BUFFER_USAGE_ ? GL_STATIC_DRAW_ : 0;
        }

        void get_buffer_parameter64(GLenum, GLenum, GLint64* params) override
        {
            *params = 0;
        }

        int live_buffers = 0;
        int live_vertex_arrays = 0;
        int live_textures = 0;
        int live_programs = 0;

    private:
        GLuint next_id_ = 1;
    };

    // Installs a FakeOglWrapper for the duration of a test case and restores
    // the previous wrapper afterwards.
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

    VertexLayout make_layout(uint16_t stride)
    {
        VertexLayout layout;
        layout.attributes.push_back(
            {AttributeSemantic::POSITION, 0,
             VertexAttributeDataType::FLOAT, 3, false, 0});
        layout.stride = stride;
        return layout;
    }
}

TEST_CASE("ResourceManager: allocations share one GL buffer")
{
    FakeGlSession session;
    ResourceManager manager;

    auto arena = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    REQUIRE(session.gl->live_buffers == 1);

    auto a = manager.allocate(arena, 4);
    auto b = manager.allocate(arena, 4);
    REQUIRE(a.arena == arena);
    REQUIRE(a.offset == 0);
    REQUIRE(a.count == 4);
    REQUIRE(b.offset == 4);
    REQUIRE(session.gl->live_buffers == 1);
}

TEST_CASE("ResourceManager: free returns a range for reuse")
{
    FakeGlSession session;
    ResourceManager manager;

    auto arena = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    auto a = manager.allocate(arena, 4);
    manager.free(a);
    REQUIRE(manager.allocate(arena, 4).offset == a.offset);
}

TEST_CASE("ResourceManager: growth preserves offsets and defers the old buffer")
{
    FakeGlSession session;
    ResourceManager manager;
    manager.begin_frame(1);

    auto arena = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 4);
    auto a = manager.allocate(arena, 4);
    auto b = manager.allocate(arena, 4); // full: grows the arena
    REQUIRE(a.offset == 0);
    REQUIRE(b.offset == 4);
    REQUIRE(manager.get_arena(arena).capacity() == 8);

    // The displaced buffer is still alive on the deletion queue until the
    // frame it was retired in has completed.
    REQUIRE(session.gl->live_buffers == 2);
    manager.collect_garbage(1);
    REQUIRE(session.gl->live_buffers == 1);
}

TEST_CASE("ResourceManager: a destroyed arena's ref is stale")
{
    FakeGlSession session;
    ResourceManager manager;

    auto arena = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    manager.destroy_arena(arena);
    REQUIRE_THROWS_AS(manager.get_arena(arena), TungstenException);
    REQUIRE_THROWS_AS(manager.allocate(arena, 4), TungstenException);
}

TEST_CASE("ResourceManager: get_vao caches per arenas and layout")
{
    FakeGlSession session;
    ResourceManager manager;

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    auto ebo = manager.create_arena(BufferUsage::STATIC_DRAW, 2, 16);
    auto layout = manager.register_layout(make_layout(12));
    std::array vbos{vbo};

    const auto vao = manager.get_vao(vbos, ebo, layout);
    REQUIRE(manager.get_vao(vbos, ebo, layout) == vao);
    REQUIRE(session.gl->live_vertex_arrays == 1);

    auto other_layout = manager.register_layout(make_layout(24));
    REQUIRE(manager.get_vao(vbos, ebo, other_layout) != vao);
    REQUIRE(session.gl->live_vertex_arrays == 2);
}

TEST_CASE("ResourceManager: registering an equal layout returns the same ref")
{
    FakeGlSession session;
    ResourceManager manager;

    auto layout = manager.register_layout(make_layout(12));
    REQUIRE(manager.register_layout(make_layout(12)) == layout);
    REQUIRE(manager.get_layout(layout).stride == 12);
}

TEST_CASE("ResourceManager: growth rebuilds the shared VAO in place")
{
    FakeGlSession session;
    ResourceManager manager;
    manager.begin_frame(1);

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 4);
    auto ebo = manager.create_arena(BufferUsage::STATIC_DRAW, 2, 16);
    auto layout = manager.register_layout(make_layout(12));
    std::array vbos{vbo};

    const auto vao = manager.get_vao(vbos, ebo, layout);
    manager.allocate(vbo, 8); // exceeds the capacity: grows the arena

    // The VAO id is unchanged; only its baked-in buffer binding moved.
    REQUIRE(manager.get_vao(vbos, ebo, layout) == vao);
    REQUIRE(session.gl->live_vertex_arrays == 1);
}

TEST_CASE("ResourceManager: destroying an arena evicts its VAOs")
{
    FakeGlSession session;
    ResourceManager manager;
    manager.begin_frame(1);

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    auto ebo = manager.create_arena(BufferUsage::STATIC_DRAW, 2, 16);
    auto layout = manager.register_layout(make_layout(12));
    std::array vbos{vbo};
    manager.get_vao(vbos, ebo, layout);

    manager.destroy_arena(vbo);
    // Both the VAO and the arena's buffer drain through the deletion queue.
    REQUIRE(session.gl->live_vertex_arrays == 1);
    REQUIRE(session.gl->live_buffers == 2);
    manager.collect_garbage(1);
    REQUIRE(session.gl->live_vertex_arrays == 0);
    REQUIRE(session.gl->live_buffers == 1); // the ebo arena's buffer
}

TEST_CASE("ResourceManager: destroying a mesh returns its slices")
{
    FakeGlSession session;
    ResourceManager manager;

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    auto ebo = manager.create_arena(BufferUsage::STATIC_DRAW, 2, 32);
    auto layout = manager.register_layout(make_layout(12));

    Mesh mesh;
    std::array vbos{vbo};
    mesh.vao = manager.get_vao(vbos, ebo, layout);
    mesh.streams = {manager.allocate(vbo, 8)};
    mesh.layout = layout;
    mesh.ebo = manager.allocate(ebo, 12);

    const auto vertex_offset = mesh.streams[0].offset;
    const auto index_offset = mesh.ebo.offset;

    auto ref = manager.create_mesh(std::move(mesh));
    REQUIRE(manager.get_mesh(ref).streams.size() == 1);

    manager.destroy_mesh(ref);
    REQUIRE_THROWS_AS(manager.get_mesh(ref), TungstenException);
    // The freed ranges are available again.
    REQUIRE(manager.allocate(vbo, 8).offset == vertex_offset);
    REQUIRE(manager.allocate(ebo, 12).offset == index_offset);
}

TEST_CASE("ResourceManager: material refs are revoked on destroy")
{
    FakeGlSession session;
    ResourceManager manager;

    Material material;
    material.parameter_data = {1, 2, 3, 4};
    auto ref = manager.create_material(std::move(material));
    REQUIRE(manager.get_material(ref).parameter_data.size() == 4);

    manager.destroy_material(ref);
    REQUIRE_THROWS_AS(manager.get_material(ref), TungstenException);
}

TEST_CASE("ResourceManager: texture deletion is deferred to frame completion")
{
    FakeGlSession session;
    ResourceManager manager;
    manager.begin_frame(7);

    Texture texture;
    texture.gl_handle = generate_texture();
    auto ref = manager.create_texture(std::move(texture));
    REQUIRE(session.gl->live_textures == 1);

    manager.destroy_texture(ref);
    // The ref is revoked immediately, the GL texture lives on.
    REQUIRE_THROWS_AS(manager.get_texture(ref), TungstenException);
    REQUIRE(session.gl->live_textures == 1);

    manager.collect_garbage(6); // an older frame: not yet safe
    REQUIRE(session.gl->live_textures == 1);
    manager.collect_garbage(7);
    REQUIRE(session.gl->live_textures == 0);
}

TEST_CASE("ResourceManager: shader variants are compiled once per key")
{
    FakeGlSession session;
    ResourceManager manager;

    ShaderFamily family;
    family.vertex_source = "#version 300 es\nvoid main() {}\n";
    family.fragment_source = "#version 300 es\nvoid main() {}\n";
    family.features = {"HAS_A", "HAS_B"};
    family.required_layout = manager.register_layout(make_layout(12));
    manager.register_shader_family(1, family);

    auto p1 = manager.register_shader_variant({1, 0b01});
    REQUIRE(manager.register_shader_variant({1, 0b01}) == p1);
    REQUIRE(session.gl->live_programs == 1);

    auto p2 = manager.register_shader_variant({1, 0b11});
    REQUIRE(!(p2 == p1));
    REQUIRE(session.gl->live_programs == 2);

    REQUIRE(manager.get_shader(p1).variant_key == ShaderVariantKey{1, 0b01});
    REQUIRE(manager.get_shader(p1).required_layout == family.required_layout);

    REQUIRE_THROWS_AS(manager.register_shader_variant({2, 0}),
                      TungstenException);
}
