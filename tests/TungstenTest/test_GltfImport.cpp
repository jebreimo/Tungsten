//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Import/GltfImport.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Resources/BuiltinShaders.hpp"

using namespace Tungsten;
using Catch::Approx;

namespace
{
    // Fabricates GL ids and reports every shader as compiled and linked, so
    // that an import can run without a GL context.
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

    private:
        GLuint next_id_ = 1;
    };

    struct FakeGlSession
    {
        FakeGlSession()
        {
            previous_ = set_custom_ogl_wrapper(
                std::make_unique<FakeOglWrapper>());
        }

        ~FakeGlSession()
        {
            set_custom_ogl_wrapper(std::move(previous_));
        }

    private:
        std::unique_ptr<IOglWrapper> previous_;
    };

    // One triangle drawn by two nodes, and two animations.
    //
    // Nodes: 0 "body" (TRS, mesh) with child 1 "arm" (TRS, no mesh); 2
    // "fixed" (a matrix, mesh); 3 "stray", in no scene.
    //
    // Animation "move" translates body linearly from 0 to 4 along x over two
    // seconds, turns arm a quarter turn about z with a step at t = 2, and has
    // three channels the importer must skip: morph weights on body, a
    // translation of the matrix node, and a translation of the stray node.
    //
    // Animation "broken" reads quaternions where a translation should be.
    //
    // The buffer holds: positions (36 bytes), indices (6 + 2 padding),
    // times {0, 2}, translations {(0,0,0), (4,0,0)}, rotations (xyzw)
    // {identity, 90° about z}, weights {0, 1}.
    constexpr const char* TEST_GLTF = R"({
  "asset": {"version": "2.0"},
  "scene": 0,
  "scenes": [{"nodes": [0, 2]}],
  "nodes": [
    {"name": "body", "translation": [1, 2, 3], "mesh": 0, "children": [1]},
    {"name": "arm", "scale": [2, 2, 2]},
    {"name": "fixed", "mesh": 0,
     "matrix": [1,0,0,0, 0,1,0,0, 0,0,1,0, 5,0,0,1]},
    {"name": "stray"}
  ],
  "meshes": [{"primitives": [{"attributes": {"POSITION": 0}, "indices": 1}]}],
  "buffers": [{"byteLength": 116, "uri": "data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAABAAIAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAgEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAPMENT/zBDU/AAAAAAAAgD8="}],
  "bufferViews": [
    {"buffer": 0, "byteOffset": 0, "byteLength": 36},
    {"buffer": 0, "byteOffset": 36, "byteLength": 6},
    {"buffer": 0, "byteOffset": 44, "byteLength": 8},
    {"buffer": 0, "byteOffset": 52, "byteLength": 24},
    {"buffer": 0, "byteOffset": 76, "byteLength": 32},
    {"buffer": 0, "byteOffset": 108, "byteLength": 8}
  ],
  "accessors": [
    {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3",
     "min": [0, 0, 0], "max": [1, 1, 0]},
    {"bufferView": 1, "componentType": 5123, "count": 3, "type": "SCALAR"},
    {"bufferView": 2, "componentType": 5126, "count": 2, "type": "SCALAR",
     "min": [0], "max": [2]},
    {"bufferView": 3, "componentType": 5126, "count": 2, "type": "VEC3"},
    {"bufferView": 4, "componentType": 5126, "count": 2, "type": "VEC4"},
    {"bufferView": 5, "componentType": 5126, "count": 2, "type": "SCALAR"}
  ],
  "animations": [
    {
      "name": "move",
      "samplers": [
        {"input": 2, "output": 3},
        {"input": 2, "output": 4, "interpolation": "STEP"},
        {"input": 2, "output": 5}
      ],
      "channels": [
        {"sampler": 0, "target": {"node": 0, "path": "translation"}},
        {"sampler": 1, "target": {"node": 1, "path": "rotation"}},
        {"sampler": 2, "target": {"node": 0, "path": "weights"}},
        {"sampler": 0, "target": {"node": 2, "path": "translation"}},
        {"sampler": 0, "target": {"node": 3, "path": "translation"}}
      ]
    },
    {
      "name": "broken",
      "samplers": [{"input": 2, "output": 4}],
      "channels": [{"sampler": 0, "target": {"node": 0, "path": "translation"}}]
    }
  ]
})";

    std::filesystem::path write_test_file()
    {
        const auto path = std::filesystem::temp_directory_path()
                          / "tungsten_test_animation.gltf";
        std::ofstream(path) << TEST_GLTF;
        return path;
    }

    bool has_warning(const GltfImport& import, std::string_view text)
    {
        return std::ranges::any_of(import.warnings, [&](const auto& warning)
        {
            return warning.find(text) != std::string::npos;
        });
    }

    struct ImportFixture
    {
        ImportFixture()
        {
            register_builtin_shader_families(resources);
            import = import_gltf(write_test_file(), resources, scene);
        }

        FakeGlSession session;
        ResourceManager resources;
        Scene scene;
        GltfImport import;
    };
}

TEST_CASE("GltfImport: animations become clips")
{
    ImportFixture f;

    REQUIRE(f.import.animations.size() == 1);
    const auto& clip = *f.import.animations[0];
    REQUIRE(clip.name == "move");
    REQUIRE(clip.duration == 2);
    // Of the five channels, only body's translation and arm's rotation can
    // be played.
    REQUIRE(clip.channels.size() == 2);
    REQUIRE(clip.channels[0].target == 0);
    REQUIRE(clip.channels[0].path == AnimationPath::TRANSLATION);
    REQUIRE(clip.channels[1].target == 1);
    REQUIRE(clip.channels[1].path == AnimationPath::ROTATION);
}

TEST_CASE("GltfImport: skipped channels and animations are reported")
{
    ImportFixture f;

    REQUIRE(has_warning(f.import, "morph target weights"));
    REQUIRE(has_warning(f.import, "'fixed', which is given as a matrix"));
    REQUIRE(has_warning(f.import, "skipped animation 'broken'"));
}

TEST_CASE("GltfImport: the targets map file nodes to scene nodes")
{
    ImportFixture f;
    const auto& targets = f.import.targets;

    REQUIRE(targets.nodes.size() == 4);
    REQUIRE(targets.rest_transforms.size() == 4);
    for (size_t i = 0; i < 3; ++i)
        REQUIRE(f.scene.is_alive(targets.nodes[i]));
    REQUIRE(!targets.nodes[3]);

    REQUIRE(f.scene.parent(targets.nodes[1]) == targets.nodes[0]);
    REQUIRE(targets.rest_transforms[0].translation == Xyz::Vector3F(1, 2, 3));
    REQUIRE(targets.rest_transforms[1].scale == Xyz::Vector3F(2, 2, 2));
}

TEST_CASE("GltfImport: an imported clip plays on the imported nodes")
{
    ImportFixture f;
    const auto& nodes = f.import.targets.nodes;
    AnimationPlayer player(f.import.animations[0], f.import.targets);

    player.set_time(1);
    player.apply(f.scene);

    // Body's translation is replaced by the animated one, halfway along.
    const auto& body = f.scene.local_matrix(nodes[0]);
    REQUIRE(body[0, 3] == Approx(2));
    REQUIRE(body[1, 3] == Approx(0));
    REQUIRE(body[2, 3] == Approx(0));

    // Arm has not stepped yet, and keeps its rest scale.
    const auto& arm_before = f.scene.local_matrix(nodes[1]);
    REQUIRE(arm_before[0, 0] == Approx(2));
    REQUIRE(arm_before[1, 0] == Approx(0).margin(1e-6));

    player.set_mode(PlaybackMode::ONCE);
    player.set_time(2);
    player.apply(f.scene);

    // A quarter turn about z takes x to y, still scaled by 2.
    const auto& arm_after = f.scene.local_matrix(nodes[1]);
    REQUIRE(arm_after[0, 0] == Approx(0).margin(1e-6));
    REQUIRE(arm_after[1, 0] == Approx(2));
}
