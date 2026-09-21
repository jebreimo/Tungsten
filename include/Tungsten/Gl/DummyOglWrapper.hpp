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
        void active_texture([[maybe_unused]] GLenum texture) override
        {
        }

        void attach_shader([[maybe_unused]] GLuint program, [[maybe_unused]] GLuint shader) override
        {
        }

        void bind_buffer([[maybe_unused]] GLenum target, [[maybe_unused]] GLuint buffer) override
        {
        }

        void bind_buffer_base([[maybe_unused]] GLenum target, [[maybe_unused]] GLuint index,
                              [[maybe_unused]] GLuint buffer) override
        {
        }

        void bind_buffer_range([[maybe_unused]] GLenum target, [[maybe_unused]] GLuint index,
                               [[maybe_unused]] GLuint buffer, [[maybe_unused]] GLintptr offset,
                               [[maybe_unused]] GLsizeiptr size) override
        {
        }

        void bind_framebuffer([[maybe_unused]] GLenum target,
                              [[maybe_unused]] GLuint framebuffer) override
        {
        }

        void bind_sampler([[maybe_unused]] GLuint unit, [[maybe_unused]] GLuint sampler) override
        {
        }

        void bind_texture([[maybe_unused]] GLenum target, [[maybe_unused]] GLuint texture) override
        {
        }

        void bind_vertex_array([[maybe_unused]] GLuint array) override
        {
        }

        void blend_color([[maybe_unused]] GLfloat red, [[maybe_unused]] GLfloat green,
                         [[maybe_unused]] GLfloat blue, [[maybe_unused]] GLfloat alpha) override
        {
        }

        void blend_equation([[maybe_unused]] GLenum mode) override
        {
        }

        void blend_equation_separate([[maybe_unused]] GLenum mode_rgb,
                                     [[maybe_unused]] GLenum mode_alpha) override
        {
        }

        void blend_func([[maybe_unused]] GLenum sFactor, [[maybe_unused]] GLenum dFactor) override
        {
        }

        void blend_func_separate([[maybe_unused]] GLenum src_rgb, [[maybe_unused]] GLenum dst_rgb,
                                 [[maybe_unused]] GLenum src_alpha,
                                 [[maybe_unused]] GLenum dst_alpha) override
        {
        }

        void buffer_data([[maybe_unused]] GLenum target, [[maybe_unused]] GLsizeiptr size,
                         [[maybe_unused]] const void* data, [[maybe_unused]] GLenum usage) override
        {
        }

        void buffer_sub_data([[maybe_unused]] GLenum target, [[maybe_unused]] GLintptr offset,
                             [[maybe_unused]] GLsizeiptr size,
                             [[maybe_unused]] const void* data) override
        {
        }

        GLenum check_framebuffer_status([[maybe_unused]] GLenum target) override
        {
            return 0;
        }

        void clear([[maybe_unused]] GLbitfield mask) override
        {
        }

        void clear_color([[maybe_unused]] GLclampf red, [[maybe_unused]] GLclampf green,
                         [[maybe_unused]] GLclampf blue, [[maybe_unused]] GLclampf alpha) override
        {
        }

        void clear_depth([[maybe_unused]] float depth) override
        {
        }

        void clear_stencil([[maybe_unused]] int32_t stencil) override
        {
        }

        void compile_shader([[maybe_unused]] GLuint shader) override
        {
        }

        void copy_buffer_sub_data([[maybe_unused]] GLenum read_target,
                                  [[maybe_unused]] GLenum write_target,
                                  [[maybe_unused]] GLintptr read_offset,
                                  [[maybe_unused]] GLintptr write_offset,
                                  [[maybe_unused]] GLsizeiptr size) override
        {
        }

        void copy_tex_sub_image_2d([[maybe_unused]] GLenum target, [[maybe_unused]] GLint level,
                                   [[maybe_unused]] GLint x_offset, [[maybe_unused]] GLint y_offset,
                                   [[maybe_unused]] GLint x, [[maybe_unused]] GLint y,
                                   [[maybe_unused]] GLsizei width,
                                   [[maybe_unused]] GLsizei height) override
        {
        }

        GLuint create_program() override
        {
            return 0;
        }

        GLuint create_shader([[maybe_unused]] GLenum type) override
        {
            return 0;
        }

        void color_mask([[maybe_unused]] GLboolean red, [[maybe_unused]] GLboolean green,
                        [[maybe_unused]] GLboolean blue, [[maybe_unused]] GLboolean alpha) override
        {
        }

        void cull_face([[maybe_unused]] GLenum mode) override
        {
        }

        void delete_buffers([[maybe_unused]] GLsizei n,
                            [[maybe_unused]] const GLuint* buffers) override
        {
        }

        void delete_framebuffers([[maybe_unused]] GLsizei n,
                                 [[maybe_unused]] const GLuint* framebuffers) override
        {
        }

        void delete_program([[maybe_unused]] GLuint program) override
        {
        }

        void delete_samplers([[maybe_unused]] GLsizei n,
                             [[maybe_unused]] const GLuint* samplers) override
        {
        }

        void delete_shader([[maybe_unused]] GLuint shader) override
        {
        }

        void delete_textures([[maybe_unused]] GLsizei n,
                             [[maybe_unused]] const GLuint* textures) override
        {
        }

        void delete_vertex_arrays([[maybe_unused]] GLsizei n,
                                  [[maybe_unused]] const GLuint* arrays) override
        {
        }

        void depth_func([[maybe_unused]] GLenum func) override
        {
        }

        void depth_mask([[maybe_unused]] GLboolean flag) override
        {
        }

        void disable([[maybe_unused]] GLenum cap) override
        {
        }

        void front_face([[maybe_unused]] GLenum mode) override
        {
        }

        void disable_vertex_attrib_array([[maybe_unused]] GLuint index) override
        {
        }

        void draw_arrays([[maybe_unused]] GLenum mode, [[maybe_unused]] GLint first,
                         [[maybe_unused]] GLsizei count) override
        {
        }

        void draw_elements([[maybe_unused]] GLenum mode, [[maybe_unused]] GLsizei count,
                           [[maybe_unused]] GLenum type,
                           [[maybe_unused]] const void* indices) override
        {
        }

        void enable([[maybe_unused]] GLenum cap) override
        {
        }

        void enable_vertex_attrib_array([[maybe_unused]] GLuint index) override
        {
        }

        void finish() override
        {
        }

        void framebuffer_texture_2d([[maybe_unused]] GLenum target,
                                    [[maybe_unused]] GLenum attachment,
                                    [[maybe_unused]] GLenum tex_target,
                                    [[maybe_unused]] GLuint texture,
                                    [[maybe_unused]] GLint level) override
        {
        }

        void gen_buffers([[maybe_unused]] GLsizei n, [[maybe_unused]] GLuint* buffers) override
        {
        }

        void generate_mipmap([[maybe_unused]] GLenum target) override
        {
        }

        void gen_framebuffers([[maybe_unused]] GLsizei n,
                              [[maybe_unused]] GLuint* framebuffers) override
        {
        }

        void gen_samplers([[maybe_unused]] GLsizei n, [[maybe_unused]] GLuint* samplers) override
        {
        }

        void gen_textures([[maybe_unused]] GLsizei n, [[maybe_unused]] GLuint* textures) override
        {
        }

        void gen_vertex_arrays([[maybe_unused]] GLsizei n, [[maybe_unused]] GLuint* arrays) override
        {
        }

        GLint get_attrib_location([[maybe_unused]] GLuint program,
                                  [[maybe_unused]] const GLchar* name) override
        {
            return 0;
        }

        void get_boolean([[maybe_unused]] GLenum pname, [[maybe_unused]] GLboolean* params) override
        {
        }

        void get_buffer_parameter([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname,
                                  [[maybe_unused]] GLint* params) override
        {
        }

        void get_buffer_parameter64([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname,
                                    [[maybe_unused]] GLint64* params) override
        {
        }

        GLenum get_error() override
        {
            return 0;
        }

        void get_float([[maybe_unused]] GLenum pname, [[maybe_unused]] GLfloat* params) override
        {
        }

        void get_integer([[maybe_unused]] GLenum pname, [[maybe_unused]] GLint* params) override
        {
        }

        void get_integer_i([[maybe_unused]] GLenum pname, [[maybe_unused]] GLuint index,
                           [[maybe_unused]] GLint* params) override
        {
        }

        void get_integer64([[maybe_unused]] GLenum pname, [[maybe_unused]] GLint64* params) override
        {
        }

        void get_program_info_log([[maybe_unused]] GLuint program,
                                  [[maybe_unused]] GLsizei buf_size,
                                  [[maybe_unused]] GLsizei* length,
                                  [[maybe_unused]] GLchar* info_log) override
        {
        }

        void get_program([[maybe_unused]] GLuint program, [[maybe_unused]] GLenum pname,
                         [[maybe_unused]] GLint* param) override
        {
        }

        void get_shader_info_log([[maybe_unused]] GLuint shader, [[maybe_unused]] GLsizei buf_size,
                                 [[maybe_unused]] GLsizei* length,
                                 [[maybe_unused]] GLchar* info_log) override
        {
        }

        void get_shader([[maybe_unused]] GLuint shader, [[maybe_unused]] GLenum pname,
                        [[maybe_unused]] GLint* param) override
        {
        }

        const GLubyte* get_string([[maybe_unused]] GLenum pname) override
        {
            return nullptr;
        }

        const GLubyte* get_string_i([[maybe_unused]] GLenum name,
                                    [[maybe_unused]] GLuint index) override
        {
            return nullptr;
        }

        void get_tex_parameter([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname,
                               [[maybe_unused]] GLfloat* params) override
        {
        }

        void get_tex_parameter([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname,
                               [[maybe_unused]] GLint* params) override
        {
        }

        GLuint get_uniform_block_index([[maybe_unused]] GLuint program,
                                       [[maybe_unused]] const GLchar* name) override
        {
            // GL_INVALID_INDEX: "no such block", so callers skip the binding.
            return 0xFFFFFFFFu;
        }

        GLint get_uniform_location([[maybe_unused]] GLuint program,
                                   [[maybe_unused]] const GLchar* name) override
        {
            return 0;
        }

        GLboolean is_buffer([[maybe_unused]] GLuint buffer) override
        {
            return 0;
        }

        GLboolean is_enabled([[maybe_unused]] GLenum cap) override
        {
            return 0;
        }

        void link_program([[maybe_unused]] GLuint program) override
        {
        }

        void sampler_parameter_f([[maybe_unused]] GLuint sampler, [[maybe_unused]] GLenum pname,
                                 [[maybe_unused]] GLfloat param) override
        {
        }

        void sampler_parameter_i([[maybe_unused]] GLuint sampler, [[maybe_unused]] GLenum pname,
                                 [[maybe_unused]] GLint param) override
        {
        }

        void shader_source([[maybe_unused]] GLuint shader, [[maybe_unused]] GLsizei count,
                           [[maybe_unused]] const GLchar* const* string,
                           [[maybe_unused]] const GLint* length) override
        {
        }

        void tex_image_2d([[maybe_unused]] GLenum target, [[maybe_unused]] GLint level,
                          [[maybe_unused]] GLint internal_format, [[maybe_unused]] GLsizei width,
                          [[maybe_unused]] GLsizei height, [[maybe_unused]] GLint border,
                          [[maybe_unused]] GLenum format, [[maybe_unused]] GLenum type,
                          [[maybe_unused]] const void* pixels) override
        {
        }

        void tex_parameter_f([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname,
                             [[maybe_unused]] GLfloat param) override
        {
        }

        void tex_parameter_i([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname,
                             [[maybe_unused]] GLint param) override
        {
        }

        void tex_storage_2d([[maybe_unused]] GLenum target, [[maybe_unused]] GLsizei levels,
                            [[maybe_unused]] GLenum internal_format, [[maybe_unused]] GLsizei width,
                            [[maybe_unused]] GLsizei height) override
        {
        }

        void tex_sub_image_2d([[maybe_unused]] GLenum target, [[maybe_unused]] GLint level,
                              [[maybe_unused]] GLint xOffset, [[maybe_unused]] GLint yOffset,
                              [[maybe_unused]] GLsizei width, [[maybe_unused]] GLsizei height,
                              [[maybe_unused]] GLenum format, [[maybe_unused]] GLenum type,
                              [[maybe_unused]] const void* pixels) override
        {
        }

        void uniform_block_binding([[maybe_unused]] GLuint program,
                                   [[maybe_unused]] GLuint block_index,
                                   [[maybe_unused]] GLuint binding) override
        {
        }

        void uniform1_f([[maybe_unused]] GLint location, [[maybe_unused]] GLfloat v0) override
        {
        }

        void uniform1_i([[maybe_unused]] GLint location, [[maybe_unused]] GLint v0) override
        {
        }

        void uniform1([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLfloat* value) override
        {
        }

        void uniform1([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLint* value) override
        {
        }

        void uniform2_f([[maybe_unused]] GLint location, [[maybe_unused]] GLfloat v0,
                        [[maybe_unused]] GLfloat v1) override
        {
        }

        void uniform2_i([[maybe_unused]] GLint location, [[maybe_unused]] GLint v0,
                        [[maybe_unused]] GLint v1) override
        {
        }

        void uniform2([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLfloat* value) override
        {
        }

        void uniform2([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLint* value) override
        {
        }

        void uniform3_f([[maybe_unused]] GLint location, [[maybe_unused]] GLfloat v0,
                        [[maybe_unused]] GLfloat v1, [[maybe_unused]] GLfloat v2) override
        {
        }

        void uniform3_i([[maybe_unused]] GLint location, [[maybe_unused]] GLint v0,
                        [[maybe_unused]] GLint v1, [[maybe_unused]] GLint v2) override
        {
        }

        void uniform3([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLfloat* value) override
        {
        }

        void uniform3([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLint* value) override
        {
        }

        void uniform4_f([[maybe_unused]] GLint location, [[maybe_unused]] GLfloat v0,
                        [[maybe_unused]] GLfloat v1, [[maybe_unused]] GLfloat v2,
                        [[maybe_unused]] GLfloat v3) override
        {
        }

        void uniform4_i([[maybe_unused]] GLint location, [[maybe_unused]] GLint v0,
                        [[maybe_unused]] GLint v1, [[maybe_unused]] GLint v2,
                        [[maybe_unused]] GLint v3) override
        {
        }

        void uniform4([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLfloat* value) override
        {
        }

        void uniform4([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                      [[maybe_unused]] const GLint* value) override
        {
        }

        void uniform_matrix2([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                             [[maybe_unused]] GLboolean transpose,
                             [[maybe_unused]] const GLfloat* value) override
        {
        }

        void uniform_matrix3([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                             [[maybe_unused]] GLboolean transpose,
                             [[maybe_unused]] const GLfloat* value) override
        {
        }

        void uniform_matrix4([[maybe_unused]] GLint location, [[maybe_unused]] GLsizei count,
                             [[maybe_unused]] GLboolean transpose,
                             [[maybe_unused]] const GLfloat* value) override
        {
        }

        void use_program([[maybe_unused]] GLuint program) override
        {
        }

        void vertex_attrib_pointer([[maybe_unused]] GLuint index, [[maybe_unused]] GLint size,
                                   [[maybe_unused]] GLenum type,
                                   [[maybe_unused]] GLboolean normalized,
                                   [[maybe_unused]] GLsizei stride,
                                   [[maybe_unused]] const void* pointer) override
        {
        }

        void scissor([[maybe_unused]] GLint x, [[maybe_unused]] GLint y,
                     [[maybe_unused]] GLsizei width, [[maybe_unused]] GLsizei height) override
        {
        }

        void invalidate_framebuffer([[maybe_unused]] GLenum target, [[maybe_unused]] GLsizei count,
                                    [[maybe_unused]] const GLenum* attachments) override
        {
        }

        void viewport([[maybe_unused]] GLint x, [[maybe_unused]] GLint y,
                      [[maybe_unused]] GLsizei width, [[maybe_unused]] GLsizei height) override
        {
        }
    };
}
