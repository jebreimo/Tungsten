//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/VertexArray.hpp"

#include <memory>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"
#include "Tungsten/Resources/VertexLayoutBuilder.hpp"
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;

namespace
{
    // One recorded attribute definition, together with the buffer that was
    // bound when it was made — which is the thing stream_index is supposed to
    // decide.
    struct AttributePointer
    {
        GLuint buffer = 0;
        GLuint location = 0;
        GLint size = 0;
        GLsizei stride = 0;
        uintptr_t offset = 0;

        bool operator==(const AttributePointer&) const = default;
    };

    class RecordingOglWrapper : public DummyOglWrapper
    {
    public:
        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
            // The dummy leaves these untouched, which would give the handle
            // an uninitialized id.
            for (GLsizei i = 0; i < n; ++i)
                arrays[i] = 1;
        }

        // The GL enums are not exposed to the tests, but the only target
        // make_vertex_array_object binds is ARRAY.
        void bind_buffer(GLenum, GLuint buffer) override
        {
            current_buffer = buffer;
        }

        void vertex_attrib_pointer(GLuint index, GLint size, GLenum,
                                   GLboolean, GLsizei stride,
                                   const void* pointer) override
        {
            pointers.push_back({current_buffer, index, size, stride,
                                reinterpret_cast<uintptr_t>(pointer)});
        }

        void enable_vertex_attrib_array(GLuint index) override
        {
            enabled.push_back(index);
        }

        GLuint current_buffer = 0;
        std::vector<AttributePointer> pointers;
        std::vector<GLuint> enabled;
    };

    struct FakeGlSession
    {
        FakeGlSession()
        {
            auto wrapper = std::make_unique<RecordingOglWrapper>();
            gl = wrapper.get();
            previous_ = set_custom_ogl_wrapper(std::move(wrapper));
        }

        ~FakeGlSession()
        {
            set_custom_ogl_wrapper(std::move(previous_));
        }

        RecordingOglWrapper* gl;

    private:
        std::unique_ptr<IOglWrapper> previous_;
    };

    // Position and normal interleaved in stream 0, texture coordinates on
    // their own in stream 1.
    VertexLayout two_stream_layout()
    {
        return VertexLayoutBuilder()
            .add_attribute(AttributeSemantic::POSITION)
            .add_attribute(AttributeSemantic::NORMAL)
            .add_attribute(AttributeSemantic::TEX_COORD_0)
            .set_stream_index(1)
            .build();
    }
}

TEST_CASE("make_vertex_array: interleaved attributes share one stride")
{
    FakeGlSession session;

    const uint32_t buffers[] = {10};
    const auto layout = VertexLayoutBuilder()
        .add_attribute(AttributeSemantic::POSITION)
        .add_attribute(AttributeSemantic::NORMAL)
        .build();
    std::ignore = make_vertex_array(buffers, layout);

    const std::vector<AttributePointer> expected = {
        {10, 0, 3, 24, 0},
        {10, 1, 3, 24, 12}
    };
    REQUIRE(session.gl->pointers == expected);
}

TEST_CASE("make_vertex_array: each attribute is read from its own stream")
{
    FakeGlSession session;

    const uint32_t buffers[] = {10, 20};
    std::ignore = make_vertex_array(buffers, two_stream_layout());

    // Locations come from the semantics: POSITION 0, NORMAL 1, TEX_COORD_0 4.
    // Stream 0's stride spans both its attributes; stream 1's spans only its
    // own, and its offset restarts at zero.
    const std::vector<AttributePointer> expected = {
        {10, 0, 3, 24, 0},
        {10, 1, 3, 24, 12},
        {20, 4, 2, 8, 0}
    };
    REQUIRE(session.gl->pointers == expected);
    REQUIRE(session.gl->enabled == std::vector<GLuint>{0, 1, 4});
}

TEST_CASE("make_vertex_array: a stream with no buffer is an error")
{
    FakeGlSession session;

    const uint32_t buffers[] = {10};
    REQUIRE_THROWS_AS(make_vertex_array(buffers, two_stream_layout()),
                      TungstenException);
}

TEST_CASE("make_vertex_array: a zero buffer id is an error")
{
    FakeGlSession session;

    const uint32_t buffers[] = {0};
    const auto layout = VertexLayoutBuilder()
        .add_attribute(AttributeSemantic::POSITION)
        .build();
    REQUIRE_THROWS_AS(make_vertex_array(buffers, layout),
                      TungstenException);
}
