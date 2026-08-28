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
#include <tuple>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "Tungsten/Neo/GlStateCache.hpp"

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

        void gen_samplers(GLsizei n, GLuint* samplers) override
        {
            for (GLsizei i = 0; i < n; ++i)
                samplers[i] = next_id_++;
            live_samplers += n;
        }

        void delete_samplers(GLsizei n, const GLuint*) override
        {
            live_samplers -= n;
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

        // Records how large each buffer store is asked to be, so the tests
        // can check that a buffer covers everything its allocator will hand
        // out.
        void buffer_data(GLenum, GLsizeiptr size, const void*,
                         GLenum) override
        {
            buffer_sizes.push_back(size);
        }

        std::vector<GLsizeiptr> buffer_sizes;
        int live_buffers = 0;
        int live_vertex_arrays = 0;
        int live_textures = 0;
        int live_samplers = 0;
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

    // A one-attribute layout. `components` is what makes two of these differ:
    // a layout is identified by what it binds, so varying the component count
    // is a real difference where varying a stride would not be one.
    VertexLayout make_layout(uint8_t components = 3)
    {
        VertexLayout layout;
        layout.attributes.push_back(
            {AttributeSemantic::POSITION, 0,
             VertexAttributeDataType::FLOAT, components, false, 0});
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

TEST_CASE("ResourceManager: a non-power-of-two arena is backed for its whole"
          " rounded-up capacity")
{
    FakeGlSession session;
    ResourceManager manager;

    // The BuddyAllocator rounds 24 up to 32 and hands out offsets across that
    // whole range, so the GL buffer has to hold 32 units. Sizing it from the
    // requested 24 would put the last two allocations past the end.
    constexpr uint16_t stride = 24;
    auto arena = manager.create_arena(BufferUsage::STATIC_DRAW, stride, 24);
    REQUIRE(manager.get_arena(arena).capacity() == 32);
    REQUIRE(session.gl->buffer_sizes.size() == 1);
    REQUIRE(session.gl->buffer_sizes[0] == 32 * stride);

    // Every offset the allocator returns must lie inside that buffer.
    for (int i = 0; i < 4; ++i)
    {
        const auto slice = manager.allocate(arena, 8);
        REQUIRE((slice.offset + slice.count) * stride
                <= uint32_t(session.gl->buffer_sizes[0]));
    }
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
    auto layout = manager.register_layout(make_layout());
    std::array vbos{vbo};

    const auto vao = manager.get_vao(vbos, ebo, layout);
    REQUIRE(manager.get_vao(vbos, ebo, layout) == vao);
    REQUIRE(session.gl->live_vertex_arrays == 1);

    auto other_layout = manager.register_layout(make_layout(2));
    REQUIRE(manager.get_vao(vbos, ebo, other_layout) != vao);
    REQUIRE(session.gl->live_vertex_arrays == 2);
}

TEST_CASE("ResourceManager: a mesh drawn with array draws needs no ebo arena")
{
    FakeGlSession session;
    ResourceManager manager;

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    auto layout = manager.register_layout(make_layout());
    std::array vbos{vbo};

    // A null ebo ref is how a non-indexed mesh is spelled; the renderer has an
    // array-draw path for exactly this, so baking its VAO must not throw.
    const auto vao = manager.get_vao(vbos, BufferArenaRef{}, layout);
    REQUIRE(vao != 0);
    REQUIRE(manager.get_vao(vbos, BufferArenaRef{}, layout) == vao);
    REQUIRE(session.gl->live_vertex_arrays == 1);
}

TEST_CASE("ResourceManager: baking a VAO announces the GL state change")
{
    FakeGlSession session;
    ResourceManager manager;

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    auto ebo = manager.create_arena(BufferUsage::STATIC_DRAW, 2, 16);
    auto layout = manager.register_layout(make_layout());
    std::array vbos{vbo};

    // Baking binds a VAO and leaves zero bound, so any GlStateCache watching
    // this context has to be told; otherwise its next bind_vao is elided
    // against a VAO that is no longer current.
    const auto before = gl_state_epoch();
    const auto vao = manager.get_vao(vbos, ebo, layout);
    REQUIRE(gl_state_epoch() != before);

    // A cache hit binds nothing, so it needs no announcement.
    const auto after_bake = gl_state_epoch();
    REQUIRE(manager.get_vao(vbos, ebo, layout) == vao);
    REQUIRE(gl_state_epoch() == after_bake);
}

TEST_CASE("ResourceManager: registering an equal layout returns the same ref")
{
    FakeGlSession session;
    ResourceManager manager;

    auto layout = manager.register_layout(make_layout());
    REQUIRE(manager.register_layout(make_layout()) == layout);
    REQUIRE(manager.get_layout(layout).attributes.size() == 1);
    REQUIRE(manager.get_layout(layout).semantics()
            == semantic_bit(AttributeSemantic::POSITION));
}

TEST_CASE("ResourceManager: one shader draws either vertex arrangement")
{
    FakeGlSession session;
    ResourceManager manager;

    // Position + normal + uv interleaved in one 32-byte stream.
    VertexLayout interleaved;
    interleaved.attributes = {
        {AttributeSemantic::POSITION, 0, VertexAttributeDataType::FLOAT, 3, false, 0},
        {AttributeSemantic::NORMAL, 0, VertexAttributeDataType::FLOAT, 3, false, 12},
        {AttributeSemantic::TEX_COORD_0, 0, VertexAttributeDataType::FLOAT, 2, false, 24},
    };

    // The same attributes, with the uv in a stream of its own.
    VertexLayout split;
    split.attributes = {
        {AttributeSemantic::POSITION, 0, VertexAttributeDataType::FLOAT, 3, false, 0},
        {AttributeSemantic::NORMAL, 0, VertexAttributeDataType::FLOAT, 3, false, 12},
        {AttributeSemantic::TEX_COORD_0, 1, VertexAttributeDataType::FLOAT, 2, false, 0},
    };

    // Different packings, so they stay distinct layouts...
    const auto a = manager.register_layout(interleaved);
    const auto b = manager.register_layout(split);
    REQUIRE(a != b);
    // ...but a shader cannot tell them apart, which is the whole point.
    REQUIRE(interleaved.semantics() == split.semantics());
    REQUIRE(interleaved.semantics()
            == (semantic_bit(AttributeSemantic::POSITION)
                | semantic_bit(AttributeSemantic::NORMAL)
                | semantic_bit(AttributeSemantic::TEX_COORD_0)));

    // Both are accepted against arenas matching their own pitches.
    const auto pnt = manager.create_arena(BufferUsage::STATIC_DRAW, 32, 16);
    const auto pn = manager.create_arena(BufferUsage::STATIC_DRAW, 24, 16);
    const auto uv = manager.create_arena(BufferUsage::STATIC_DRAW, 8, 16);

    Mesh one;
    one.layout = a;
    one.streams = {manager.allocate(pnt, 4)};
    const auto mesh_a = manager.create_mesh(std::move(one));

    Mesh two;
    two.layout = b;
    two.streams = {manager.allocate(pn, 4), manager.allocate(uv, 4)};
    const auto mesh_b = manager.create_mesh(std::move(two));

    REQUIRE(manager.get_mesh(mesh_a).semantics
            == manager.get_mesh(mesh_b).semantics);
}

TEST_CASE("ResourceManager: create_mesh rejects an attribute past its vertex")
{
    FakeGlSession session;
    ResourceManager manager;

    VertexLayout layout_value;
    // 12 bytes of data at offset 4 needs a 16-byte vertex.
    layout_value.attributes.push_back(
        {AttributeSemantic::POSITION, 0,
         VertexAttributeDataType::FLOAT, 3, false, 4});
    const auto layout = manager.register_layout(layout_value);

    const auto tight = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);
    Mesh overrun;
    overrun.layout = layout;
    overrun.streams = {manager.allocate(tight, 4)};
    REQUIRE_THROWS_WITH(
        manager.create_mesh(std::move(overrun)),
        Catch::Matchers::ContainsSubstring("past the end"));

    const auto roomy = manager.create_arena(BufferUsage::STATIC_DRAW, 16, 16);
    Mesh fits;
    fits.layout = layout;
    fits.streams = {manager.allocate(roomy, 4)};
    REQUIRE_NOTHROW(manager.create_mesh(std::move(fits)));
}

TEST_CASE("ResourceManager: create_mesh rejects a stream the mesh lacks")
{
    FakeGlSession session;
    ResourceManager manager;

    VertexLayout layout_value;
    layout_value.attributes = {
        {AttributeSemantic::POSITION, 0, VertexAttributeDataType::FLOAT, 3, false, 0},
        {AttributeSemantic::TEX_COORD_0, 1, VertexAttributeDataType::FLOAT, 2, false, 0},
    };
    const auto layout = manager.register_layout(layout_value);
    const auto arena = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 16);

    Mesh mesh;
    mesh.layout = layout;
    mesh.streams = {manager.allocate(arena, 4)}; // only one, layout wants two
    REQUIRE_THROWS_WITH(
        manager.create_mesh(std::move(mesh)),
        Catch::Matchers::ContainsSubstring(
            "reads a vertex stream the mesh does not have"));
}

TEST_CASE("ResourceManager: growth rebuilds the shared VAO in place")
{
    FakeGlSession session;
    ResourceManager manager;
    manager.begin_frame(1);

    auto vbo = manager.create_arena(BufferUsage::STATIC_DRAW, 12, 4);
    auto ebo = manager.create_arena(BufferUsage::STATIC_DRAW, 2, 16);
    auto layout = manager.register_layout(make_layout());
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
    auto layout = manager.register_layout(make_layout());
    std::array vbos{vbo};
    std::ignore = manager.get_vao(vbos, ebo, layout);

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
    auto layout = manager.register_layout(make_layout());

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

    using B = std::byte;
    Material material;
    material.parameter_data = {B{1}, B{2}, B{3}, B{4}};
    auto ref = manager.create_material(std::move(material));
    REQUIRE(manager.get_material(ref).parameter_data.size() == 4);

    manager.destroy_material(ref);
    REQUIRE_THROWS_AS(manager.get_material(ref), TungstenException);
}

TEST_CASE("ResourceManager: a material's parameters go into a buffer of its own")
{
    FakeGlSession session;
    ResourceManager manager;

    using B = std::byte;
    Material material;
    material.parameter_data = {B{1}, B{2}, B{3}, B{4}};
    const auto ref = manager.create_material(std::move(material));

    // One buffer, uploaded once, at creation — not on every draw.
    REQUIRE(session.gl->live_buffers == 1);
    REQUIRE(manager.get_material(ref).ubo);
    REQUIRE(session.gl->buffer_sizes == std::vector<GLsizeiptr>{4});

    // A second material gets a second buffer, so the two never share state.
    Material other;
    other.parameter_data = {B{9}};
    const auto other_ref = manager.create_material(std::move(other));
    REQUIRE(session.gl->live_buffers == 2);
    REQUIRE(manager.get_material(other_ref).ubo.id()
            != manager.get_material(ref).ubo.id());
}

TEST_CASE("ResourceManager: a material without parameters gets no buffer")
{
    FakeGlSession session;
    ResourceManager manager;

    const auto ref = manager.create_material(Material{});
    REQUIRE(!manager.get_material(ref).ubo);
    REQUIRE(session.gl->live_buffers == 0);
}

TEST_CASE("ResourceManager: updating a material's parameters re-uploads them")
{
    FakeGlSession session;
    ResourceManager manager;

    using B = std::byte;
    const auto ref = manager.create_material(Material{});
    REQUIRE(!manager.get_material(ref).ubo);

    // The first update on a parameterless material has to create the buffer.
    const std::vector params = {B{1}, B{2}};
    manager.update_material_parameters(ref, params);
    REQUIRE(manager.get_material(ref).ubo);
    REQUIRE(manager.get_material(ref).parameter_data.size() == 2);
    const auto buffer_id = manager.get_material(ref).ubo.id();

    // A later update reuses the same buffer rather than making another.
    const std::vector larger = {B{1}, B{2}, B{3}, B{4}};
    manager.update_material_parameters(ref, larger);
    REQUIRE(manager.get_material(ref).ubo.id() == buffer_id);
    REQUIRE(session.gl->live_buffers == 1);
    REQUIRE(session.gl->buffer_sizes == std::vector<GLsizeiptr>{2, 4});
}

TEST_CASE("ResourceManager: destroying a material defers its buffer's deletion")
{
    FakeGlSession session;
    ResourceManager manager;
    manager.begin_frame(3);

    using B = std::byte;
    Material material;
    material.parameter_data = {B{1}, B{2}, B{3}, B{4}};
    const auto ref = manager.create_material(std::move(material));
    REQUIRE(session.gl->live_buffers == 1);

    manager.destroy_material(ref);
    // The ref is revoked at once; the GL buffer waits for the frame to pass.
    REQUIRE_THROWS_AS(manager.get_material(ref), TungstenException);
    REQUIRE(session.gl->live_buffers == 1);

    manager.collect_garbage(3);
    REQUIRE(session.gl->live_buffers == 0);
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
    manager.register_shader_family(1, family);

    auto p1 = manager.register_shader_variant({1, 0b01});
    REQUIRE(manager.register_shader_variant({1, 0b01}) == p1);
    REQUIRE(session.gl->live_programs == 1);

    auto p2 = manager.register_shader_variant({1, 0b11});
    REQUIRE(!(p2 == p1));
    REQUIRE(session.gl->live_programs == 2);

    REQUIRE(manager.get_shader(p1).variant_key == ShaderVariantKey{1, 0b01});
    REQUIRE(manager.get_shader(p1).required_attributes
            == family.required_attributes);

    REQUIRE_THROWS_AS(manager.register_shader_variant({2, 0}),
                      TungstenException);
}

TEST_CASE("ResourceManager: re-registering a family recompiles its variants")
{
    FakeGlSession session;
    ResourceManager manager;

    ShaderFamily family;
    family.vertex_source = "#version 300 es\nvoid main() {}\n";
    family.fragment_source = "#version 300 es\nvoid main() {}\n";
    manager.register_shader_family(1, family);

    const auto first = manager.register_shader_variant({1, 0});
    REQUIRE(session.gl->live_programs == 1);

    // A hot reload: the same id, new sources. The variant compiled from the
    // old sources must not be handed out again.
    family.fragment_source = "#version 300 es\nvoid main() { /* v2 */ }\n";
    manager.register_shader_family(1, family);

    const auto second = manager.register_shader_variant({1, 0});
    REQUIRE(!(second == first));
    REQUIRE(session.gl->live_programs == 2);

    // The old program is deliberately still resolvable: a Material holding
    // that ref keeps working until it is pointed at the new variant.
    REQUIRE(manager.get_shader(first).variant_key == ShaderVariantKey{1, 0});
}

TEST_CASE("ResourceManager: compiling a variant with samplers announces the"
          " GL state change")
{
    FakeGlSession session;
    ResourceManager manager;

    ShaderFamily family;
    family.vertex_source = "#version 300 es\nvoid main() {}\n";
    family.fragment_source = "#version 300 es\nvoid main() {}\n";
    // Pointing the samplers at their units needs the program bound, and there
    // is nothing to restore it to afterwards — so the caches must be told.
    family.samplers = {"u_diffuse"};
    manager.register_shader_family(1, family);

    const auto before = gl_state_epoch();
    manager.register_shader_variant({1, 0});
    REQUIRE(gl_state_epoch() != before);

    // The second call is a cache hit: nothing is compiled, nothing is bound.
    const auto after_compile = gl_state_epoch();
    manager.register_shader_variant({1, 0});
    REQUIRE(gl_state_epoch() == after_compile);
}

TEST_CASE("ResourceManager: equal sampler descriptors are interned")
{
    FakeGlSession session;
    ResourceManager manager;

    constexpr SamplerDescriptor clamped{
        .mip_filter = SamplerMipFilter::NONE,
        .address_mode_u = SamplerAddressMode::CLAMP_TO_EDGE,
        .address_mode_v = SamplerAddressMode::CLAMP_TO_EDGE
    };

    const auto first = manager.register_sampler(clamped);
    REQUIRE(session.gl->live_samplers == 1);

    // An equal descriptor is the same sampler, not a second GL object.
    const auto again = manager.register_sampler(clamped);
    REQUIRE(again == first);
    REQUIRE(manager.get_sampler_id(again) == manager.get_sampler_id(first));
    REQUIRE(session.gl->live_samplers == 1);

    auto repeating = clamped;
    repeating.address_mode_u = SamplerAddressMode::REPEAT;
    const auto other = manager.register_sampler(repeating);
    REQUIRE_FALSE(other == first);
    REQUIRE(session.gl->live_samplers == 2);
}

TEST_CASE("ResourceManager: a null sampler ref resolves to the default sampler")
{
    FakeGlSession session;
    ResourceManager manager;

    // Nothing is created until a sampler is actually asked for.
    REQUIRE(session.gl->live_samplers == 0);

    const auto id = manager.get_sampler_id({});
    REQUIRE(id != 0);
    REQUIRE(session.gl->live_samplers == 1);
    // The default is interned like any other, so resolving it again is free.
    REQUIRE(manager.get_sampler_id({}) == id);
    REQUIRE(manager.get_sampler_id(manager.default_sampler()) == id);
    REQUIRE(session.gl->live_samplers == 1);
}

TEST_CASE("ResourceManager: the default sampler expects no mipmaps")
{
    FakeGlSession session;
    ResourceManager manager;

    // A single-level texture — the renderer's white dummy among them — would be
    // incomplete if the default asked for mipmaps, which SamplerDescriptor's
    // own defaults do.
    const auto& descriptor = manager.get_sampler_descriptor(manager.default_sampler());
    REQUIRE(descriptor.mip_filter == SamplerMipFilter::NONE);
}

TEST_CASE("ResourceManager: a garbage sampler ref throws")
{
    FakeGlSession session;
    ResourceManager manager;

    manager.register_sampler({});
    REQUIRE_THROWS_AS(manager.get_sampler_id({99, 1}), TungstenException);
    REQUIRE_THROWS_AS(manager.get_sampler_descriptor({0, 99}), TungstenException);
}
