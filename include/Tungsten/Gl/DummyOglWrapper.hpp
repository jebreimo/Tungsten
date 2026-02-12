//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "IOglWrapper.hpp"

namespace Tungsten
{
    class DummyOglWrapper : public IOglWrapper
    {
    public:
        void active_texture(GLenum texture) override
        {
        }

        void attach_shader(GLuint program, GLuint shader) override
        {
        }

        void bind_buffer(GLenum target, GLuint buffer) override
        {
        }

        void bind_framebuffer(GLenum target, GLuint framebuffer) override
        {
        }

        void bind_texture(GLenum target, GLuint texture) override
        {
        }

        void bind_vertex_array(GLuint array) override
        {
        }

        void blend_func(GLenum sFactor, GLenum dFactor) override
        {
        }

        void buffer_data(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override
        {
        }

        void buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size,
                             const void* data) override
        {
        }

        GLenum check_framebuffer_status(GLenum target) override
        {
            return 0;
        }

        void clear(GLbitfield mask) override
        {
        }

        void clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
        {
        }

        void clear_depth(float depth) override
        {
        }

        void clear_stencil(int32_t stencil) override
        {
        }

        void compile_shader(GLuint shader) override
        {
        }

        void copy_tex_sub_image_2d(GLenum target, GLint level, GLint x_offset, GLint y_offset,
                                   GLint x, GLint y, GLsizei width, GLsizei height) override
        {
        }

        GLuint create_program() override
        {
            return 0;
        }

        GLuint create_shader(GLenum type) override
        {
            return 0;
        }

        void cull_face(GLenum mode) override
        {
        }

        void delete_buffers(GLsizei n, const GLuint* buffers) override
        {
        }

        void delete_framebuffers(GLsizei n, const GLuint* framebuffers) override
        {
        }

        void delete_program(GLuint program) override
        {
        }

        void delete_shader(GLuint shader) override
        {
        }

        void delete_textures(GLsizei n, const GLuint* textures) override
        {
        }

        void delete_vertex_arrays(GLsizei n, const GLuint* arrays) override
        {
        }

        void disable(GLenum cap) override
        {
        }

        void disable_vertex_attrib_array(GLuint index) override
        {
        }

        void draw_arrays(GLenum mode, GLint first, GLsizei count) override
        {
        }

        void draw_elements(GLenum mode, GLsizei count, GLenum type, const void* indices) override
        {
        }

        void enable(GLenum cap) override
        {
        }

        void enable_vertex_attrib_array(GLuint index) override
        {
        }

        void finish() override
        {
        }

        void framebuffer_texture_2d(GLenum target, GLenum attachment, GLenum tex_target,
                                    GLuint texture, GLint level) override
        {
        }

        void gen_buffers(GLsizei n, GLuint* buffers) override
        {
        }

        void generate_mipmap(GLenum target) override
        {
        }

        void gen_framebuffers(GLsizei n, GLuint* framebuffers) override
        {
        }

        void gen_textures(GLsizei n, GLuint* textures) override
        {
        }

        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
        }

        GLint get_attrib_location(GLuint program, const GLchar* name) override
        {
            return 0;
        }

        void get_buffer_parameter(GLenum target, GLenum pname, GLint* params) override
        {
        }

        GLenum get_error() override
        {
            return 0;
        }

        void get_integer(GLenum pname, GLint* params) override
        {
        }

        void get_program_info_log(GLuint program, GLsizei buf_size, GLsizei* length,
                                  GLchar* info_log) override
        {
        }

        void get_program(GLuint program, GLenum pname, GLint* param) override
        {
        }

        void get_shader_info_log(GLuint shader, GLsizei buf_size, GLsizei* length,
                                 GLchar* info_log) override
        {
        }

        void get_shader(GLuint shader, GLenum pname, GLint* param) override
        {
        }

        const GLubyte* get_string(GLenum pname) override
        {
            return nullptr;
        }

        const GLubyte* get_string_i(GLenum name, GLuint index) override
        {
            return nullptr;
        }

        void get_tex_parameter(GLenum target, GLenum pname, GLfloat* params) override
        {
        }

        void get_tex_parameter(GLenum target, GLenum pname, GLint* params) override
        {
        }

        GLint get_uniform_location(GLuint program, const GLchar* name) override
        {
            return 0;
        }

        GLboolean is_buffer(GLuint buffer) override
        {
            return 0;
        }

        GLboolean is_enabled(GLenum cap) override
        {
            return 0;
        }

        void link_program(GLuint program) override
        {
        }

        void shader_source(GLuint shader, GLsizei count, const GLchar* const* string,
                           const GLint* length) override
        {
        }

        void tex_image_2d(GLenum target, GLint level, GLint internal_format, GLsizei width,
                          GLsizei height, GLint border, GLenum format, GLenum type,
                          const void* pixels) override
        {
        }

        void tex_parameter_f(GLenum target, GLenum pname, GLfloat param) override
        {
        }

        void tex_parameter_i(GLenum target, GLenum pname, GLint param) override
        {
        }

        void tex_storage_2d(GLenum target, GLsizei levels, GLenum internal_format, GLsizei width,
                            GLsizei height) override
        {
        }

        void tex_sub_image_2d(GLenum target, GLint level, GLint xOffset, GLint yOffset,
                              GLsizei width, GLsizei height, GLenum format, GLenum type,
                              const void* pixels) override
        {
        }

        void uniform1_f(GLint location, GLfloat v0) override
        {
        }

        void uniform1_i(GLint location, GLint v0) override
        {
        }

        void uniform1(GLint location, GLsizei count, const GLfloat* value) override
        {
        }

        void uniform1(GLint location, GLsizei count, const GLint* value) override
        {
        }

        void uniform2_f(GLint location, GLfloat v0, GLfloat v1) override
        {
        }

        void uniform2_i(GLint location, GLint v0, GLint v1) override
        {
        }

        void uniform2(GLint location, GLsizei count, const GLfloat* value) override
        {
        }

        void uniform2(GLint location, GLsizei count, const GLint* value) override
        {
        }

        void uniform3_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override
        {
        }

        void uniform3_i(GLint location, GLint v0, GLint v1, GLint v2) override
        {
        }

        void uniform3(GLint location, GLsizei count, const GLfloat* value) override
        {
        }

        void uniform3(GLint location, GLsizei count, const GLint* value) override
        {
        }

        void uniform4_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
        {
        }

        void uniform4_i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override
        {
        }

        void uniform4(GLint location, GLsizei count, const GLfloat* value) override
        {
        }

        void uniform4(GLint location, GLsizei count, const GLint* value) override
        {
        }

        void uniform_matrix2(GLint location, GLsizei count, GLboolean transpose,
                             const GLfloat* value) override
        {
        }

        void uniform_matrix3(GLint location, GLsizei count, GLboolean transpose,
                             const GLfloat* value) override
        {
        }

        void uniform_matrix4(GLint location, GLsizei count, GLboolean transpose,
                             const GLfloat* value) override
        {
        }

        void use_program(GLuint program) override
        {
        }

        void vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                                   GLsizei stride, const void* pointer) override
        {
        }

        void viewport(GLint x, GLint y, GLsizei width, GLsizei height) override
        {
        }
    };
}
