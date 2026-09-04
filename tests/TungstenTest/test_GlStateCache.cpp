//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/GlStateCache.hpp"

#include <memory>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Gl/DummyOglWrapper.hpp"

using namespace Tungsten;

namespace
{
    // Counts the binds that actually reach GL, which is the only thing the
    // cache is supposed to affect.
    class CountingOglWrapper : public DummyOglWrapper
    {
    public:
        void use_program(GLuint program) override
        {
            programs.push_back(program);
        }

        void bind_vertex_array(GLuint array) override
        {
            vaos.push_back(array);
        }

        std::vector<GLuint> programs;
        std::vector<GLuint> vaos;
    };

    struct FakeGlSession
    {
        FakeGlSession()
        {
            auto wrapper = std::make_unique<CountingOglWrapper>();
            gl = wrapper.get();
            previous_ = set_custom_ogl_wrapper(std::move(wrapper));
        }

        ~FakeGlSession()
        {
            set_custom_ogl_wrapper(std::move(previous_));
        }

        CountingOglWrapper* gl;

    private:
        std::unique_ptr<IOglWrapper> previous_;
    };
}

TEST_CASE("GlStateCache: a repeated bind is elided")
{
    FakeGlSession session;
    GlStateCache cache;

    cache.bind_vao(7);
    cache.bind_vao(7);
    cache.use_program(3);
    cache.use_program(3);

    REQUIRE(session.gl->vaos == std::vector<GLuint>{7});
    REQUIRE(session.gl->programs == std::vector<GLuint>{3});
}

TEST_CASE("GlStateCache: a fresh cache assumes nothing is bound")
{
    FakeGlSession session;
    GlStateCache cache;

    // Zero is a real binding, not a synonym for "already in that state" — a
    // cache created mid-session has no idea what the context has bound.
    cache.bind_vao(0);
    cache.use_program(0);

    REQUIRE(session.gl->vaos == std::vector<GLuint>{0});
    REQUIRE(session.gl->programs == std::vector<GLuint>{0});
}

TEST_CASE("GlStateCache: notify_gl_state_changed forces the next bind")
{
    FakeGlSession session;
    GlStateCache cache;

    cache.bind_vao(7);
    cache.use_program(3);
    REQUIRE(session.gl->vaos.size() == 1);

    // Something bound behind the cache's back — a VAO bake, a variant
    // compile — and left GL somewhere else entirely.
    notify_gl_state_changed();

    cache.bind_vao(7);
    cache.use_program(3);
    REQUIRE(session.gl->vaos == std::vector<GLuint>{7, 7});
    REQUIRE(session.gl->programs == std::vector<GLuint>{3, 3});

    // And the cache is current again afterwards.
    cache.bind_vao(7);
    REQUIRE(session.gl->vaos.size() == 2);
}

TEST_CASE("GlStateCache: every cache over the context hears the notification")
{
    FakeGlSession session;
    GlStateCache first;
    GlStateCache second;

    first.bind_vao(7);
    second.bind_vao(7);
    REQUIRE(session.gl->vaos.size() == 2);

    notify_gl_state_changed();

    first.bind_vao(7);
    second.bind_vao(7);
    REQUIRE(session.gl->vaos.size() == 4);
}

TEST_CASE("GlStateCache: invalidate re-issues even a bind of zero")
{
    FakeGlSession session;
    GlStateCache cache;

    cache.bind_vao(0);
    cache.invalidate();
    cache.bind_vao(0);

    // Two calls: after invalidate the cache must not confuse "unknown" with
    // "already zero".
    REQUIRE(session.gl->vaos == std::vector<GLuint>{0, 0});
}