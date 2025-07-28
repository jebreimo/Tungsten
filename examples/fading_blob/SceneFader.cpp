//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SceneFader.hpp"

#include "Tungsten/ArrayBufferBuilder.hpp"
#include "Tungsten/GlFramebuffer.hpp"
#include "Tungsten/GlTexture.hpp"
#include "Tungsten/GlVertices.hpp"
#include "Tungsten/ShaderProgramBuilder.hpp"

namespace
{
    const char* VERTEX_SHADER = R"(
        #version 330 core
        in highp vec2 a_position;
        in highp vec2 a_tex_position;
        out highp vec2 u_tex_position;
        void main()
        {
            u_tex_position = a_tex_position;
            gl_Position = vec4(a_position, 0.0, 1.0);
        })";

    const char* FRAGMENT_SHADER = R"(
        #version 330 core
        in highp vec2 u_tex_position;
        uniform sampler2D u_texture;
        uniform highp vec3 u_color_delta;
        out highp vec4 fragColor;
        void main()
        {
            highp vec4 tex_color = texture(u_texture, u_tex_position);
            highp vec3 sum = tex_color.rgb + u_color_delta;
            highp vec3 result = vec3(max(sum.r, 0.0),
                                     max(sum.g, 0.0),
                                     max(sum.b, 0.0));
            fragColor = vec4(result, 1.0);
        })";

    class TextureFaderProgram
    {
    public:
        TextureFaderProgram()
        {
            program = Tungsten::ShaderProgramBuilder()
                .add_shader(Tungsten::ShaderType::VERTEX, VERTEX_SHADER)
                .add_shader(Tungsten::ShaderType::FRAGMENT, FRAGMENT_SHADER)
                .build();

            position_attr = get_vertex_attribute(program, "a_position");
            tex_position_attr = get_vertex_attribute(program, "a_tex_position");
            texture = Tungsten::get_uniform<GLint>(program, "u_texture");
            color_delta = Tungsten::get_uniform<Xyz::Vector3F>(program, "u_color_delta");
        }

        Tungsten::ProgramHandle program;

        GLuint position_attr;
        GLuint tex_position_attr;
        Tungsten::Uniform<GLint> texture;
        Tungsten::Uniform<Xyz::Vector3F> color_delta;
    };

    struct TextureFaderVertex
    {
        Xyz::Vector2F pos;
        Xyz::Vector2F tex_pos;
    };
}

class SceneFader::SceneFaderImpl
{
public:
    explicit SceneFaderImpl(Tungsten::Size2I window_size)
    {
        frame_buffer_ = Tungsten::generate_framebuffer();

        Tungsten::generate_textures(textures_);
        set_window_size(window_size);

        Tungsten::ArrayBuffer<TextureFaderVertex> buffer;
        Tungsten::ArrayBufferBuilder builder(buffer);
        builder.add_vertex({{-1, -1}, {0, 0}})
            .add_vertex({{1, -1}, {1, 0}})
            .add_vertex({{1, 1}, {1, 1}})
            .add_vertex({{-1, 1}, {0, 1}});
        builder.add_indexes(0, 1, 2)
            .add_indexes(0, 2, 3);
        Tungsten::set_buffers(vertex_array_, buffer);
        vertex_array_.bind();
        vertex_array_.define_float_pointer(program_.position_attr, 2, 0);
        vertex_array_.define_float_pointer(program_.tex_position_attr, 2, 2 * sizeof(float));
    }

    void set_window_size(Tungsten::Size2I size)
    {
        for (auto& texture : textures_)
        {
            Tungsten::bind_texture(GL_TEXTURE_2D, texture);
            Tungsten::set_texture_image_2d(GL_TEXTURE_2D, 0, GL_RGB, size,
                                           Tungsten::RGB_TEXTURE);
            Tungsten::set_min_filter(GL_TEXTURE_2D, GL_LINEAR);
            Tungsten::set_mag_filter(GL_TEXTURE_2D, GL_LINEAR);
        }
    }

    void draw_previous_scene(float fadeout)
    {
        Tungsten::bind_framebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_);
        Tungsten::framebuffer_texture_2d(GL_DRAW_FRAMEBUFFER,
                                         GL_COLOR_ATTACHMENT0,
                                         GL_TEXTURE_2D,
                                         textures_[index_]);

        Tungsten::activate_texture_unit(0);
        Tungsten::bind_texture(GL_TEXTURE_2D, textures_[1 - index_]);
        use_program(program_.program);
        program_.texture.set(0);
        program_.color_delta.set({-1.f / 256.f, -1.f / 256.f, -1.f / 256.f});
        vertex_array_.bind();
        Tungsten::draw_triangle_elements_16(0, 6);
    }

    void render_scene()
    {
        Tungsten::bind_framebuffer(GL_DRAW_FRAMEBUFFER, 0);
        Tungsten::activate_texture_unit(0);
        Tungsten::bind_texture(GL_TEXTURE_2D, textures_[1 - index_]);
        use_program(program_.program);
        program_.texture.set(0);
        program_.color_delta.set({0.f, 0.f, 0.f});
        vertex_array_.bind();
        Tungsten::draw_triangle_elements_16(0, 6);

        index_ = 1 - index_;
    }

private:
    Xyz::Vector3F color_delta_ = {0, 0, 0};
    Tungsten::FramebufferHandle frame_buffer_;
    std::array<Tungsten::TextureHandle, 2> textures_;
    Tungsten::VertexArray<TextureFaderVertex> vertex_array_;
    TextureFaderProgram program_;
    int index_ = 0;
};

SceneFader::SceneFader(Tungsten::Size2I window_size)
    : impl_(std::make_unique<SceneFaderImpl>(window_size))
{
}

SceneFader::~SceneFader() = default;

void SceneFader::set_window_size(Tungsten::Size2I window_size)
{
    impl_->set_window_size(window_size);
}

void SceneFader::draw_previous_scene(float fadeout)
{
    impl_->draw_previous_scene(fadeout);
}

void SceneFader::render_scene()
{
    impl_->render_scene();
}
