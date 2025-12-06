//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>

using GLenum = unsigned int;
using GLuint = unsigned int;
using GLbitfield = unsigned int;
using GLint = int;
using GLsizei = int;
using GLintptr = ptrdiff_t;
using GLsizeiptr = ptrdiff_t;
using GLclampf = float;
using GLchar = char;
using GLubyte = unsigned char;
using GLboolean = unsigned char;
using GLfloat = float;

namespace Tungsten
{
    class IOglWrapper
    {
    public:
        virtual ~IOglWrapper() = default;

        virtual void active_texture(GLenum texture) = 0;

        virtual void attach_shader(GLuint program, GLuint shader) = 0;

        virtual void bind_buffer(GLenum target, GLuint buffer) = 0;

        virtual void bind_framebuffer(GLenum target, GLuint framebuffer) = 0;

        virtual void bind_texture(GLenum target, GLuint texture) = 0;

        virtual void bind_vertex_array(GLuint array) = 0;

        virtual void blend_func(GLenum sFactor, GLenum dFactor) = 0;

        virtual void buffer_data(GLenum target, GLsizeiptr size,
                                 const void* data, GLenum usage) = 0;

        virtual void buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size,
                                     const void* data) = 0;

        virtual GLenum check_framebuffer_status(GLenum target) = 0;

        virtual void clear(GLbitfield mask) = 0;

        virtual void clear_color(GLclampf red, GLclampf green,
                                 GLclampf blue, GLclampf alpha) = 0;

        virtual void clear_depth(float depth) = 0;

        virtual void clear_stencil(int32_t stencil) = 0;

        virtual void compile_shader(GLuint shader) = 0;

        virtual void copy_tex_sub_image_2d(GLenum target, GLint level,
                                           GLint x_offset, GLint y_offset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height) = 0;

        virtual GLuint create_program() = 0;

        virtual GLuint create_shader(GLenum type) = 0;

        virtual void cull_face(GLenum mode) = 0;

        virtual void delete_buffers(GLsizei n, const GLuint* buffers) = 0;

        virtual void delete_framebuffers(GLsizei n, const GLuint* framebuffers) = 0;

        virtual void delete_program(GLuint program) = 0;

        virtual void delete_shader(GLuint shader) = 0;

        virtual void delete_textures(GLsizei n, const GLuint* textures) = 0;

        virtual void delete_vertex_arrays(GLsizei n, const GLuint* arrays) = 0;

        virtual void disable(GLenum cap) = 0;

        virtual void disable_vertex_attrib_array(GLuint index) = 0;

        virtual void draw_arrays(GLenum mode, GLint first, GLsizei count) = 0;

        virtual void draw_elements(GLenum mode, GLsizei count,
                                   GLenum type, const void* indices) = 0;

        virtual void enable(GLenum cap) = 0;

        virtual void enable_vertex_attrib_array(GLuint index) = 0;

        virtual void finish() = 0;

        virtual void framebuffer_texture_2d(GLenum target,
                                            GLenum attachment,
                                            GLenum tex_target,
                                            GLuint texture,
                                            GLint level) = 0;

        virtual void gen_buffers(GLsizei n, GLuint* buffers) = 0;

        virtual void generate_mipmap(GLenum target) = 0;

        virtual void gen_framebuffers(GLsizei n, GLuint* framebuffers) = 0;

        virtual void gen_textures(GLsizei n, GLuint* textures) = 0;

        virtual void gen_vertex_arrays(GLsizei n, GLuint* arrays) = 0;

        virtual GLint get_attrib_location(GLuint program, const GLchar* name) = 0;

        virtual void get_buffer_parameter(GLenum target, GLenum pname, GLint* params) = 0;

        virtual GLenum getError() = 0;

        virtual void get_integer(GLenum pname, GLint* params) = 0;

        virtual void get_program_info_log(GLuint program, GLsizei buf_size,
                                          GLsizei* length, GLchar* info_log) = 0;

        virtual void get_program(GLuint program, GLenum pname, GLint* param) = 0;

        virtual void get_shader_info_log(GLuint shader, GLsizei buf_size,
                                         GLsizei* length, GLchar* info_log) = 0;

        virtual void get_shader(GLuint shader, GLenum pname, GLint* param) = 0;

        virtual const GLubyte* get_string(GLenum name) = 0;

        virtual void get_tex_parameter(GLenum target, GLenum pname, GLfloat* params) = 0;

        virtual void get_tex_parameter(GLenum target, GLenum pname, GLint* params) = 0;

        virtual GLint get_uniform_location(GLuint program, const GLchar* name) = 0;

        virtual GLboolean is_buffer(GLuint buffer) = 0;

        virtual GLboolean is_enabled(GLenum cap) = 0;

        virtual void link_program(GLuint program) = 0;

        virtual void shader_source(GLuint shader, GLsizei count,
                                   const GLchar* const* string,
                                   const GLint* length) = 0;

        virtual void tex_image_2d(GLenum target, GLint level,
                                  GLint internal_format, GLsizei width,
                                  GLsizei height,
                                  GLint border, GLenum format,
                                  GLenum type, const void* pixels) = 0;

        virtual void tex_parameter_f(GLenum target, GLenum pname, GLfloat param) = 0;

        virtual void tex_parameter_i(GLenum target, GLenum pname, GLint param) = 0;

        virtual void tex_storage_2d(GLenum target, GLsizei levels, GLenum internal_format,
                                    GLsizei width,
                                    GLsizei height) = 0;

        virtual void tex_sub_image_2d(GLenum target, GLint level,
                                      GLint xOffset, GLint yOffset,
                                      GLsizei width,
                                      GLsizei height, GLenum format, GLenum type,
                                      const void* pixels) = 0;

        virtual void uniform1_f(GLint location, GLfloat v0) = 0;

        virtual void uniform1_i(GLint location, GLint v0) = 0;

        virtual void uniform1(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform1(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform2_f(GLint location, GLfloat v0, GLfloat v1) = 0;

        virtual void uniform2_i(GLint location, GLint v0, GLint v1) = 0;

        virtual void uniform2(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform2(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform3_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = 0;

        virtual void uniform3_i(GLint location, GLint v0, GLint v1, GLint v2) = 0;

        virtual void uniform3(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform3(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform4_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = 0;

        virtual void uniform4_i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) = 0;

        virtual void uniform4(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform4(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform_matrix2(GLint location, GLsizei count, GLboolean transpose,
                                        const GLfloat* value) = 0;

        virtual void uniform_matrix3(GLint location, GLsizei count, GLboolean transpose,
                                        const GLfloat* value) = 0;

        virtual void uniform_matrix4(GLint location, GLsizei count, GLboolean transpose,
                                        const GLfloat* value) = 0;

        virtual void use_program(GLuint program) = 0;

        virtual void vertex_attrib_pointer(GLuint index, GLint size, GLenum type,
                                           GLboolean normalized, GLsizei stride,
                                           const void* pointer) = 0;

        virtual void viewport(GLint x, GLint y, GLsizei width, GLsizei height) = 0;
    };

    enum class StandardOglWrapper
    {
        DEFAULT,
        DUMMY
    };

    IOglWrapper& get_ogl_wrapper();

    [[nodiscard]] bool is_ogl_tracing_enabled();

    void set_ogl_tracing_enabled(bool enabled);

    std::unique_ptr<IOglWrapper> set_standard_ogl_wrapper(StandardOglWrapper wrapper_type);

    std::unique_ptr<IOglWrapper> set_custom_ogl_wrapper(std::unique_ptr<IOglWrapper> wrapper);
}
