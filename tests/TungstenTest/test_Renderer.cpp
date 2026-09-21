//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Rendering/Renderer.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/SceneGraph/CameraComponent.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/Mesh.hpp"
#include "Tungsten/SceneGraph/NodeHandle.hpp"
#include "Tungsten/SceneGraph/RenderableComponent.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"
#include "Tungsten/Rendering/SnapshotBuilder.hpp"
#include "Tungsten/Resources/Texture.hpp"
#include "Tungsten/Resources/VertexLayout.hpp"
#include "Tungsten/TungstenException.hpp"
#include "../../src/Tungsten/Resources/UboBindings.hpp"

using namespace Tungsten;

namespace
{
    // The fixed-function state a draw runs with. Mirrored from the GL calls
    // and snapshotted at each draw, so a test can say what a draw was
    // configured with without pinning the order the calls arrived in — the
    // binder is free to reorder or elide them.
    struct DrawState
    {
        bool depth_test = false;
        bool depth_write = false;
        bool blend = false;
        bool cull = false;
        GLenum depth_func = 0;
        GLenum blend_src = 0;
        GLenum blend_dst = 0;
        GLenum cull_mode = 0;

        bool operator==(const DrawState&) const = default;
    };

    // Fabricates GL ids and records the calls the renderer's behavior is
    // specified by: draws (with their index offsets, to observe ordering),
    // program binds, buffer uploads (by size, to tell the three UBOs apart),
    // uniform-block bindings, and the fixed-function state of each draw.
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

        void use_program([[maybe_unused]] GLuint program) override
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

        // glBindSampler names its unit explicitly rather than going through
        // the active-texture selector, so record the unit it was given.
        void gen_samplers(GLsizei n, GLuint* samplers) override
        {
            for (GLsizei i = 0; i < n; ++i)
                samplers[i] = next_id_++;
        }

        void bind_sampler(GLuint unit, GLuint sampler) override
        {
            sampler_binds.emplace_back(static_cast<int>(unit), sampler);
        }

        void buffer_data(GLenum, GLsizeiptr size, const void*, GLenum) override
        {
            buffer_uploads.push_back(static_cast<size_t>(size));
        }

        // Materials own their parameter buffers, so switching material
        // re-points binding 1 instead of uploading. Recording those binds is
        // how the tests observe material batching.
        void bind_buffer_base(GLenum, GLuint index, GLuint buffer) override
        {
            if (index == PER_MATERIAL_UBO_BINDING)
                material_ubo_binds.push_back(buffer);
        }

        // The per-draw blocks go up in one packed upload, and each draw binds
        // its own slice of it.
        void bind_buffer_range(GLenum, GLuint index, GLuint,
                               GLintptr offset, GLsizeiptr size) override
        {
            if (index == PER_DRAW_UBO_BINDING)
                per_draw_binds.emplace_back(offset, size);
        }

        // A realistic alignment, so the tests exercise the padding the
        // renderer has to insert between blocks.
        void get_integer(GLenum pname, GLint* params) override
        {
            constexpr GLenum GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT_ = 0x8A34;
            *params = pname == GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT_
                          ? UBO_ALIGNMENT : 0;
        }

        void draw_elements(GLenum, GLsizei count, GLenum,
                           const void* indices) override
        {
            events.push_back("draw@" + std::to_string(
                reinterpret_cast<intptr_t>(indices)));
            draw_states.push_back(current);
            ++draw_calls;
            last_draw_count = count;
        }

        void draw_arrays(GLenum, GLint first, GLsizei) override
        {
            events.push_back("draw_arrays@" + std::to_string(first));
            draw_states.push_back(current);
            ++draw_calls;
        }

        void clear(GLbitfield mask) override
        {
            clear_masks.push_back(mask);
        }

        void clear_color(GLclampf red, GLclampf green,
                         GLclampf blue, GLclampf alpha) override
        {
            last_clear_color = {red, green, blue, alpha};
        }

        void viewport(GLint x, GLint y, GLsizei width, GLsizei height) override
        {
            viewports.emplace_back(x, y, width, height);
        }

        void bind_framebuffer(GLenum, GLuint framebuffer) override
        {
            framebuffer_binds.push_back(framebuffer);
        }

        void depth_mask(GLboolean flag) override
        {
            events.emplace_back(flag ? "depth_write_on" : "depth_write_off");
            current.depth_write = flag != 0;
            ++state_calls;
        }

        void depth_func(GLenum func) override
        {
            current.depth_func = func;
            ++state_calls;
        }

        void blend_func_separate(GLenum src_rgb, GLenum dst_rgb,
                                 GLenum, GLenum) override
        {
            current.blend_src = src_rgb;
            current.blend_dst = dst_rgb;
            ++state_calls;
        }

        void cull_face(GLenum mode) override
        {
            current.cull_mode = mode;
            ++state_calls;
        }

        void enable(GLenum cap) override
        {
            if (cap == 0x0BE2) // GL_BLEND
            {
                events.emplace_back("blend_on");
                current.blend = true;
            }
            else if (cap == 0x0B71) // GL_DEPTH_TEST
            {
                current.depth_test = true;
            }
            else if (cap == 0x0B44) // GL_CULL_FACE
            {
                current.cull = true;
            }
            ++state_calls;
        }

        void disable(GLenum cap) override
        {
            if (cap == 0x0BE2) // GL_BLEND
            {
                events.emplace_back("blend_off");
                current.blend = false;
            }
            else if (cap == 0x0B71) // GL_DEPTH_TEST
            {
                current.depth_test = false;
            }
            else if (cap == 0x0B44) // GL_CULL_FACE
            {
                current.cull = false;
            }
            ++state_calls;
        }

        DrawState current;
        std::vector<DrawState> draw_states;
        std::vector<GLbitfield> clear_masks;
        std::array<GLclampf, 4> last_clear_color{};
        std::vector<std::tuple<GLint, GLint, GLsizei, GLsizei>> viewports;
        std::vector<GLuint> framebuffer_binds;
        /** Every fixed-function call, to observe redundant-toggle elision. */
        int state_calls = 0;
        std::vector<std::pair<GLuint, GLuint>> block_bindings;
        std::vector<std::pair<GLint, GLint>> int_uniforms;
        std::vector<std::pair<int, GLuint>> texture_binds;
        std::vector<std::pair<int, GLuint>> sampler_binds;
        static constexpr GLint UBO_ALIGNMENT = 256;

        std::vector<size_t> buffer_uploads;
        std::vector<GLuint> material_ubo_binds;
        std::vector<std::pair<GLintptr, GLsizeiptr>> per_draw_binds;
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

            layout = resources.register_layout(layout_value);

            vbo_arena = resources.create_arena(BufferUsage::STATIC_DRAW,
                                               12, 256);
            ebo_arena = resources.create_arena(BufferUsage::STATIC_DRAW,
                                               2, 256);

            ShaderFamily family;
            family.vertex_source = "#version 300 es\nvoid main() {}\n";
            family.fragment_source = "#version 300 es\nvoid main() {}\n";
            resources.register_shader_family(1, family);
            shader = resources.register_shader_variant({1, 0});

            pipeline = make_pipeline(shader);

            Material material_value;
            material_value.pipeline = pipeline;
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
            scene.resolve_transforms();
            SnapshotBuilder(resources)
                .build(scene, camera_node.id(), snapshot);
            Renderer renderer(resources);
            // The constructor allocates the UBOs through buffer_data and
            // creates the white texture; reset the recording so the tests
            // observe only render().
            gl.buffer_uploads.clear();
            gl.events.clear();
            gl.texture_binds.clear();
            gl.sampler_binds.clear();
            gl.material_ubo_binds.clear();
            gl.per_draw_binds.clear();
            gl.draw_states.clear();
            gl.state_calls = 0;
            gl.clear_masks.clear();
            gl.viewports.clear();
            gl.framebuffer_binds.clear();
            renderer.render(snapshot, pass);
        }

        ResourceManager resources;
        Scene scene;
        NodeHandle camera_node;
        RenderSnapshot snapshot;
        // Draws to the window, clearing colour and depth: the default a
        // single-pass application would write.
        RenderPassDescriptor pass{.viewport = {{0, 0}, {640, 480}}};
        // Registers a pipeline over the bench's layout. `transparent` gives
        // the blended, no-depth-write state the back-to-front pass wants.
        PipelineRef make_pipeline(ShaderProgramRef program,
                                  bool transparent = false)
        {
            PipelineDescriptor descriptor;
            descriptor.shader = program;
            descriptor.layout = layout;
            if (transparent)
            {
                descriptor.queue = RenderQueue::TRANSPARENT;
                descriptor.depth.write = false;
                descriptor.blend = {
                    .enabled = true,
                    .src_color = BlendFunction::SRC_ALPHA,
                    .dst_color = BlendFunction::ONE_MINUS_SRC_ALPHA,
                    .src_alpha = BlendFunction::SRC_ALPHA,
                    .dst_alpha = BlendFunction::ONE_MINUS_SRC_ALPHA
                };
            }
            return resources.register_pipeline(descriptor);
        }

        VertexLayoutRef layout;
        BufferArenaRef vbo_arena;
        BufferArenaRef ebo_arena;
        ShaderProgramRef shader;
        PipelineRef pipeline;
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
    family.samplers = {{"u_diffuse_map"}, {"u_specular_map"}};
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
    family.samplers = {{"u_diffuse_map"}, {"u_specular_map"}};
    bench.resources.register_shader_family(2, family);

    Material material_value;
    material_value.pipeline = bench.make_pipeline(
        bench.resources.register_shader_variant({2, 0}));
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

    // And both get the default sampler: a unit left without one would sample
    // through whatever another subsystem last bound to it.
    const auto& samplers = session.gl->sampler_binds;
    REQUIRE(samplers.size() == 2);
    REQUIRE(samplers[0].first == 0);
    REQUIRE(samplers[1].first == 1);
    REQUIRE(samplers[0].second != 0);
    REQUIRE(samplers[0].second
            == bench.resources.get_sampler_id(bench.resources.default_sampler()));
}

TEST_CASE("Renderer: a texture is drawn with the sampler it names")
{
    FakeGlSession session;
    Bench bench;

    ShaderFamily family;
    family.vertex_source = "#version 300 es\nvoid main() {}\n";
    family.fragment_source = "#version 300 es\nvoid main() {}\n";
    family.samplers = {{"u_diffuse_map"}};
    bench.resources.register_shader_family(3, family);

    const auto sampler = bench.resources.register_sampler(
        {.address_mode_u = SamplerAddressMode::MIRRORED_REPEAT});

    Texture texture_value;
    texture_value.gl_handle = generate_texture();
    texture_value.sampler = sampler;
    const auto texture = bench.resources.create_texture(
        std::move(texture_value));

    Material material_value;
    material_value.pipeline = bench.make_pipeline(
        bench.resources.register_shader_variant({3, 0}));
    material_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
    material_value.textures = {texture};
    const auto material = bench.resources.create_material(
        std::move(material_value));
    bench.add_renderable(bench.make_mesh(4, 6), material, -10);

    bench.build_and_render(*session.gl);

    const auto& samplers = session.gl->sampler_binds;
    REQUIRE(samplers.size() == 1);
    REQUIRE(samplers[0].first == 0);
    REQUIRE(samplers[0].second == bench.resources.get_sampler_id(sampler));
    // Not the default: the texture named its own.
    REQUIRE(samplers[0].second
            != bench.resources.get_sampler_id(bench.resources.default_sampler()));
}

TEST_CASE("Renderer: a material with no parameters for a shader that reads"
          " them is rejected")
{
    const FakeGlSession session;
    Bench bench;

    // parameter_data left empty. Drawing it would show whatever the previous
    // material uploaded, or an empty UBO on the first draw of the frame.
    Material bare;
    bare.pipeline = bench.pipeline;
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

    // The per-draw blocks go up together, once, rather than one upload per
    // item: two items, two binds, one packed upload spanning both slots.
    const auto& binds = session.gl->per_draw_binds;
    REQUIRE(binds.size() == 2);
    REQUIRE(count_uploads(session.gl->buffer_uploads,
                          2 * FakeOglWrapper::UBO_ALIGNMENT) == 1);
    REQUIRE(count_uploads(session.gl->buffer_uploads, PER_DRAW_SIZE) == 0);

    // Each item binds its own slice, sized to the block, at an offset the
    // driver will accept.
    REQUIRE(binds[0].second == GLsizeiptr(PER_DRAW_SIZE));
    REQUIRE(binds[1].second == GLsizeiptr(PER_DRAW_SIZE));
    REQUIRE(binds[0].first == 0);
    REQUIRE(binds[1].first == FakeOglWrapper::UBO_ALIGNMENT);
    for (const auto& [offset, size] : binds)
        REQUIRE(offset % FakeOglWrapper::UBO_ALIGNMENT == 0);
}

TEST_CASE("Renderer: binds program and material once for a batched run")
{
    const FakeGlSession session;
    Bench bench;
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -20);
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -30);

    bench.build_and_render(*session.gl);
    // All three items share one material: one glUseProgram, one UBO bind — and
    // no upload at all, the parameters went up when the material was created.
    REQUIRE(session.gl->program_binds == 1);
    REQUIRE(session.gl->material_ubo_binds.size() == 1);
    REQUIRE(count_uploads(session.gl->buffer_uploads, MATERIAL_BLOB_SIZE) == 0);
}

TEST_CASE("Renderer: re-binds the material when it changes between items")
{
    const FakeGlSession session;
    Bench bench;
    Material second_value;
    second_value.pipeline = bench.pipeline;
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
    REQUIRE(session.gl->material_ubo_binds.size() == 2);
    // And the two binds name different buffers — each material has its own.
    REQUIRE(session.gl->material_ubo_binds[0]
            != session.gl->material_ubo_binds[1]);
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
    transparent_value.pipeline = bench.make_pipeline(bench.shader, true);
    transparent_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
    const auto transparent = bench.resources.create_material(
        std::move(transparent_value));

    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, transparent, -10);
    bench.add_renderable(mesh, bench.material, -20);

    bench.build_and_render(*session.gl);

    // Two draws: the opaque one first, unblended, then the transparent one
    // with SRC_ALPHA / ONE_MINUS_SRC_ALPHA. Asserted as the state each draw
    // ran with, not as a sequence of toggles — which toggles the binder emits
    // to get there is its business.
    const auto& draws = session.gl->draw_states;
    REQUIRE(draws.size() == 2);
    REQUIRE_FALSE(draws[0].blend);
    REQUIRE(draws[1].blend);
    REQUIRE(draws[1].blend_src == GLenum(0x0302));  // GL_SRC_ALPHA
    REQUIRE(draws[1].blend_dst == GLenum(0x0303));  // GL_ONE_MINUS_SRC_ALPHA
}

TEST_CASE("Renderer: a transparent pipeline draws without writing depth")
{
    const FakeGlSession session;
    Bench bench;
    Material transparent_value;
    transparent_value.pipeline = bench.make_pipeline(bench.shader, true);
    transparent_value.parameter_data.resize(MATERIAL_BLOB_SIZE);
    const auto transparent = bench.resources.create_material(
        std::move(transparent_value));

    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, transparent, -10);
    bench.add_renderable(mesh, bench.material, -20);

    bench.build_and_render(*session.gl);

    // The blended draw keeps the depth *test* but drops the write, so it is
    // still occluded by opaque geometry and never occludes anything coplanar
    // behind it. This is what TextSystem's pipeline relies on.
    const auto& draws = session.gl->draw_states;
    REQUIRE(draws.size() == 2);
    REQUIRE(draws[0].depth_test);
    REQUIRE(draws[0].depth_write);
    REQUIRE(draws[1].depth_test);
    REQUIRE_FALSE(draws[1].depth_write);
}

TEST_CASE("Renderer: items sharing a pipeline emit no redundant state calls")
{
    const FakeGlSession session;
    Bench bench;
    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, bench.material, -10);
    bench.build_and_render(*session.gl);
    const int after_one = session.gl->state_calls;

    // A second item on the same pipeline must cost nothing in state: the
    // binder only issues what differs from the pipeline bound before it.
    const FakeGlSession second_session;
    Bench second;
    const auto second_mesh = second.make_mesh(4, 6);
    second.add_renderable(second_mesh, second.material, -10);
    second.add_renderable(second_mesh, second.material, -20);
    second.build_and_render(*second_session.gl);

    REQUIRE(second_session.gl->draw_states.size() == 2);
    REQUIRE(second_session.gl->state_calls == after_one);
}

TEST_CASE("Renderer: differing pipelines each get the state they asked for")
{
    const FakeGlSession session;
    Bench bench;

    // Two pipelines differing only in raster state, both opaque, so they sort
    // into one run and the binder has to switch between them mid-pass.
    PipelineDescriptor unculled;
    unculled.shader = bench.shader;
    unculled.layout = bench.layout;
    unculled.raster.cull_enabled = false;

    Material unculled_material;
    unculled_material.pipeline = bench.resources.register_pipeline(unculled);
    unculled_material.parameter_data.resize(MATERIAL_BLOB_SIZE);
    const auto material = bench.resources.create_material(
        std::move(unculled_material));

    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, bench.material, -10);
    bench.add_renderable(mesh, material, -20);

    bench.build_and_render(*session.gl);

    const auto& draws = session.gl->draw_states;
    REQUIRE(draws.size() == 2);
    REQUIRE(draws[0].cull != draws[1].cull);
}

TEST_CASE("Renderer: an all-opaque frame keeps depth writes on throughout")
{
    const FakeGlSession session;
    Bench bench;
    const auto mesh = bench.make_mesh(4, 6);
    bench.add_renderable(mesh, bench.material, -10);

    bench.build_and_render(*session.gl);

    for (const auto& event : session.gl->events)
        REQUIRE(event != "depth_write_off");
}

TEST_CASE("Renderer: a mesh providing more than its shader reads is fine")
{
    const FakeGlSession session;
    Bench bench;
    // bench.layout is POSITION only and the family requires nothing, so a
    // superset draws. The complementary case — a layout missing something the
    // shader reads — is now rejected at register_pipeline, so it lives in
    // test_PipelineRegistry.cpp rather than here.
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);
    REQUIRE_NOTHROW(bench.build_and_render(*session.gl));
}

TEST_CASE("Renderer: a mesh without an index buffer uses an array draw")
{
    const FakeGlSession session;
    Bench bench;
    Mesh mesh_value;
    mesh_value.streams = {bench.resources.allocate(bench.vbo_arena, 8)};
    mesh_value.layout = bench.layout;
    // ebo left null: not an indexed mesh.
    const auto mesh = bench.resources.create_mesh(std::move(mesh_value));
    bench.add_renderable(mesh, bench.material, -10);

    bench.build_and_render(*session.gl);
    REQUIRE(session.gl->draw_calls == 1);
    REQUIRE(session.gl->events.back().starts_with("draw_arrays@"));
}

TEST_CASE("Renderer: a CLEAR pass clears colour and depth")
{
    const FakeGlSession session;
    Bench bench;
    bench.pass.color.clear_color = {0.25f, 0.5f, 0.75f, 1.0f};
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);

    bench.build_and_render(*session.gl);

    constexpr GLbitfield COLOR_BUFFER_BIT = 0x00004000;
    constexpr GLbitfield DEPTH_BUFFER_BIT = 0x00000100;
    REQUIRE(session.gl->clear_masks.size() == 1);
    REQUIRE((session.gl->clear_masks[0] & COLOR_BUFFER_BIT) != 0);
    REQUIRE((session.gl->clear_masks[0] & DEPTH_BUFFER_BIT) != 0);
    REQUIRE(session.gl->last_clear_color[2] == 0.75f);
}

TEST_CASE("Renderer: a LOAD pass clears nothing")
{
    const FakeGlSession session;
    Bench bench;
    // What is already in the target is the point — an accumulation buffer, or
    // anything drawn by an earlier pass.
    bench.pass.color.load = LoadAction::LOAD;
    bench.pass.depth->load = LoadAction::LOAD;
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);

    bench.build_and_render(*session.gl);

    REQUIRE(session.gl->clear_masks.empty());
}

TEST_CASE("Renderer: the pass issues its viewport and binds its target")
{
    const FakeGlSession session;
    Bench bench;
    bench.pass.viewport = {{10, 20}, {320, 240}};
    bench.add_renderable(bench.make_mesh(4, 6), bench.material, -10);

    bench.build_and_render(*session.gl);

    REQUIRE(session.gl->viewports.size() == 1);
    REQUIRE(session.gl->viewports[0]
            == std::tuple<GLint, GLint, GLsizei, GLsizei>{10, 20, 320, 240});
    // A null target is the window's framebuffer.
    REQUIRE_FALSE(session.gl->framebuffer_binds.empty());
    REQUIRE(session.gl->framebuffer_binds[0] == 0u);
}
