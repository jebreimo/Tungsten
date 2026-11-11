//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-11.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Import/ObjImport.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Resources/BuiltinShaders.hpp"
#include "Tungsten/Resources/Material.hpp"
#include "Tungsten/Resources/PipelineDescriptor.hpp"
#include "Tungsten/SceneGraph/NodeHandle.hpp"

using namespace Tungsten;

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

    // A red quad in shape "front", which the reader splits into two
    // triangles, and a triangle in shape "back" that uses no material. The
    // quad has normals and texture coordinates, the triangle neither.
    constexpr const char* TEST_OBJ = R"(mtllib tungsten_test_import.mtl
v 0 0 0
v 2 0 0
v 2 1 0
v 0 1 0
v 0 0 -3
vn 0 0 1
vt 0 0
vt 1 0
vt 1 1
vt 0 1
o front
usemtl red
f 1/1/1 2/2/1 3/3/1 4/4/1
o back
usemtl
f 1 4 5
)";

    // "red" has a diffuse texture that does not exist; "glass" is half
    // transparent.
    constexpr const char* TEST_MTL = R"(newmtl red
Kd 1 0 0
Ks 0.5 0.5 0.5
Ns 2000
map_Kd tungsten_missing_texture.png
newmtl glass
Kd 1 1 1
d 0.5
)";

    // A quad using "glass" and a triangle using "red", in the same shape.
    constexpr const char* TWO_MATERIALS_OBJ = R"(mtllib tungsten_test_import.mtl
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
usemtl glass
f 1 2 3 4
usemtl red
f 1 3 4
)";

    constexpr const char* LINES_OBJ = R"(v 0 0 0
v 1 0 0
l 1 2
)";

    std::filesystem::path write_test_file(const char* name, const char* text)
    {
        const auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream(path) << text;
        return path;
    }

    bool has_warning(const ObjImport& import, std::string_view text)
    {
        return std::ranges::any_of(import.warnings, [&](const auto& warning)
        {
            return warning.find(text) != std::string::npos;
        });
    }

    struct ImportFixture
    {
        explicit ImportFixture(const char* obj = TEST_OBJ,
                               const ObjImportOptions& options = {})
        {
            register_builtin_shader_families(resources);
            write_test_file("tungsten_test_import.mtl", TEST_MTL);
            import = import_obj(
                write_test_file("tungsten_test_import.obj", obj),
                resources, scene, {}, options);
        }

        const PipelineDescriptor& pipeline_of(size_t renderable) const
        {
            const auto& component =
                scene.components<RenderableComponent>().items[renderable];
            const auto& material = resources.get_material(component.material);
            return resources.get_pipeline(material.pipeline);
        }

        FakeGlSession session;
        ResourceManager resources;
        Scene scene;
        ObjImport import;
    };
}

TEST_CASE("ObjImport: each shape becomes a renderable under the root")
{
    ImportFixture f;

    REQUIRE(f.import.primitive_count == 2);
    const auto& renderables = f.scene.components<RenderableComponent>();
    REQUIRE(renderables.items.size() == 2);
    for (const auto node : f.scene.children(f.import.root))
        REQUIRE(f.scene.find_component<RenderableComponent>(node.id()));
    REQUIRE(f.import.animations.empty());
}

TEST_CASE("ObjImport: the bounds cover every face")
{
    ImportFixture f;

    REQUIRE(f.import.bounds.min == Xyz::Vector3F(0, 0, -3));
    REQUIRE(f.import.bounds.max == Xyz::Vector3F(2, 1, 0));
}

TEST_CASE("ObjImport: a missing texture is a warning, not an error")
{
    ImportFixture f;

    REQUIRE(has_warning(f.import, "tungsten_missing_texture.png"));
}

TEST_CASE("ObjImport: each material in a shape gets its own renderable")
{
    ImportFixture f(TWO_MATERIALS_OBJ);

    REQUIRE(f.import.primitive_count == 2);
    // Ordered by material id: glass is the second material in the file,
    // red the first.
    REQUIRE(f.pipeline_of(0).queue == RenderQueue::OPAQUE);
    REQUIRE(f.pipeline_of(1).queue == RenderQueue::TRANSPARENT);
    REQUIRE(f.pipeline_of(1).blend.enabled);
}

TEST_CASE("ObjImport: wireframes are made on request")
{
    ImportFixture f(TEST_OBJ, {.create_wireframes = true});

    REQUIRE(f.import.wireframes.size() == 2);
    for (const auto& wireframe : f.import.wireframes)
    {
        const auto& material = f.resources.get_material(wireframe.material);
        const auto& pipeline = f.resources.get_pipeline(material.pipeline);
        REQUIRE(pipeline.primitive == TopologyType::LINES);
    }
}

TEST_CASE("ObjImport: a file without faces is refused")
{
    FakeGlSession session;
    ResourceManager resources;
    register_builtin_shader_families(resources);
    Scene scene;

    const auto path = write_test_file("tungsten_test_lines.obj", LINES_OBJ);
    REQUIRE_THROWS_AS(import_obj(path, resources, scene), TungstenException);
    // Nothing is left behind but the scene's own root.
    REQUIRE(scene.components<RenderableComponent>().items.empty());
}
