//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/PipelineDescriptor.hpp"

#include <memory>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Resources/ResourceManager.hpp"
#include "Tungsten/Resources/ShaderFamily.hpp"
#include "Tungsten/Resources/VertexLayout.hpp"
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;

namespace
{
    // Enough of a GL to let ShaderLibrary compile and link: unique ids and a
    // success answer to every status query.
    class FakeOglWrapper : public DummyOglWrapper
    {
    public:
        GLuint create_shader(GLenum) override { return next_id_++; }

        GLuint create_program() override { return next_id_++; }

        const GLubyte* get_string(GLenum name) override
        {
            if (name == 0x8B8C) // GL_SHADING_LANGUAGE_VERSION
                return reinterpret_cast<const GLubyte*>("3.00");
            if (name == 0x1F02) // GL_VERSION
                return reinterpret_cast<const GLubyte*>("OpenGL ES 3.0");
            return nullptr;
        }

        void get_shader(GLuint, GLenum, GLint* param) override { *param = 1; }

        void get_program(GLuint, GLenum, GLint* param) override { *param = 1; }

    private:
        GLuint next_id_ = 1;
    };

    struct FakeGlSession
    {
        FakeGlSession()
        {
            auto wrapper = std::make_unique<FakeOglWrapper>();
            previous_ = set_custom_ogl_wrapper(std::move(wrapper));
        }

        ~FakeGlSession()
        {
            set_custom_ogl_wrapper(std::move(previous_));
        }

        std::unique_ptr<IOglWrapper> previous_;
    };

    VertexLayout make_layout(AttributeSemantic semantic
                                 = AttributeSemantic::POSITION)
    {
        VertexLayout layout;
        layout.attributes.push_back(
            {semantic, 0, VertexAttributeDataType::FLOAT, 3, false, 0});
        return layout;
    }

    ShaderProgramRef register_shader(ResourceManager& manager,
                                     AttributeSemanticMask required = 0)
    {
        ShaderFamily family;
        family.vertex_source = "#version 300 es\nvoid main() {}\n";
        family.fragment_source = "#version 300 es\nvoid main() {}\n";
        family.required_attributes = required;
        manager.register_shader_family(1, family);
        return manager.register_shader_variant({1, 0});
    }

    PipelineDescriptor make_descriptor(ResourceManager& manager)
    {
        PipelineDescriptor descriptor;
        descriptor.shader = register_shader(manager);
        descriptor.layout = manager.register_layout(make_layout());
        return descriptor;
    }
}

TEST_CASE("PipelineRegistry: equal descriptors intern to one ref")
{
    FakeGlSession session;
    ResourceManager manager;

    const auto descriptor = make_descriptor(manager);
    const auto ref = manager.register_pipeline(descriptor);

    REQUIRE(manager.register_pipeline(descriptor) == ref);
    REQUIRE(manager.get_pipeline(ref) == descriptor);
}

TEST_CASE("PipelineRegistry: any differing state makes a distinct pipeline")
{
    FakeGlSession session;
    ResourceManager manager;

    const auto base = make_descriptor(manager);
    const auto ref = manager.register_pipeline(base);

    // One case per state group, so a field dropped out of operator== shows up
    // here rather than as two draws silently sharing one pipeline.
    auto blended = base;
    blended.blend.enabled = true;
    REQUIRE(manager.register_pipeline(blended) != ref);

    auto no_depth_write = base;
    no_depth_write.depth.write = false;
    REQUIRE(manager.register_pipeline(no_depth_write) != ref);

    auto unculled = base;
    unculled.raster.cull_enabled = false;
    REQUIRE(manager.register_pipeline(unculled) != ref);

    auto lines = base;
    lines.primitive = TopologyType::LINES;
    REQUIRE(manager.register_pipeline(lines) != ref);

    auto transparent = base;
    transparent.queue = RenderQueue::TRANSPARENT;
    REQUIRE(manager.register_pipeline(transparent) != ref);
}

TEST_CASE("PipelineRegistry: a layout missing what the shader reads throws")
{
    FakeGlSession session;
    ResourceManager manager;

    // The layout provides POSITION; the shader also reads NORMAL. Registering
    // the pipeline is where the two are named together, so this is where the
    // mismatch has to be reported — not on each draw.
    PipelineDescriptor descriptor;
    descriptor.shader = register_shader(
        manager, semantic_bit(AttributeSemantic::POSITION)
                     | semantic_bit(AttributeSemantic::NORMAL));
    descriptor.layout = manager.register_layout(make_layout());

    REQUIRE_THROWS_WITH(
        manager.register_pipeline(descriptor),
        Catch::Matchers::ContainsSubstring(
            "does not provide every vertex attribute"));
}

TEST_CASE("PipelineRegistry: a layout providing more than is read is fine")
{
    FakeGlSession session;
    ResourceManager manager;

    // Only the semantics a shader actually reads have to be present.
    PipelineDescriptor descriptor;
    descriptor.shader = register_shader(manager);
    descriptor.layout = manager.register_layout(make_layout());

    REQUIRE_NOTHROW(manager.register_pipeline(descriptor));
}

TEST_CASE("PipelineRegistry: a null or stale ref throws rather than aliasing")
{
    FakeGlSession session;
    ResourceManager manager;

    std::ignore = manager.register_pipeline(make_descriptor(manager));

    REQUIRE_THROWS_AS(manager.get_pipeline(PipelineRef{}), TungstenException);
    REQUIRE_THROWS_AS(manager.get_pipeline(PipelineRef{99, 1}),
                      TungstenException);
}
