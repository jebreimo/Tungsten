//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-28.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <format>
#include <iostream>

#include "Tungsten/IOglWrapper.hpp"
#include <GL/glew.h>

namespace Tungsten
{
    class TracingOglWrapper final : public IOglWrapper
    {
    public:
        template <class... Args>
        void log(std::format_string<Args...> fmt, Args&&... args)
        {
            auto str = std::vformat(fmt.get(), std::make_format_args(args...));
            std::clog << str << std::endl;
        }

        void active_texture(GLenum texture) override
        {
            log("glActiveTexture({})", texture);
            wrapper->active_texture(texture);
        }

        void attach_shader(GLuint program, GLuint shader) override
        {
            log("glAttachShader({}, {})", program, shader);
            wrapper->attach_shader(program, shader);
        }

        void bind_buffer(GLenum target, GLuint buffer) override
        {
            log("glBindBuffer({}, {})", target, buffer);
            wrapper->bind_buffer(target, buffer);
        }

        void bind_framebuffer(GLenum target, GLuint framebuffer) override
        {
            log("glBindFramebuffer({}, {})", target, framebuffer);
            wrapper->bind_framebuffer(target, framebuffer);
        }

        void bind_texture(GLenum target, GLuint texture) override
        {
            log("glBindTexture({}, {})", target, texture);
            wrapper->bind_texture(target, texture);
        }

        void bind_vertex_array(GLuint array) override
        {
            log("glBindVertexArray({})", array);
            wrapper->bind_vertex_array(array);
        }

        void blend_func(GLenum sFactor, GLenum dFactor) override
        {
            log("glBlendFunc({}, {})", sFactor, dFactor);
            wrapper->blend_func(sFactor, dFactor);
        }

        void buffer_data(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override
        {
            log("glBufferData({}, {}, {}, {})", target, size, static_cast<const void*>(data),
                usage);
            wrapper->buffer_data(target, size, data, usage);
        }

        void buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size,
                             const void* data) override
        {
            log("glBufferSubData({}, {}, {}, {})", target, offset, size,
                static_cast<const void*>(data));
            wrapper->buffer_sub_data(target, offset, size, data);
        }

        GLenum check_framebuffer_status(GLenum target) override
        {
            log("glCheckFramebufferStatus({})", target);
            return wrapper->check_framebuffer_status(target);
        }

        void clear(GLbitfield mask) override
        {
            log("glClear({:#x})", mask);
            wrapper->clear(mask);
        }

        void clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
        {
            log("glClearColor({}, {}, {}, {})", red, green, blue, alpha);
            wrapper->clear_color(red, green, blue, alpha);
        }

        void clear_depth(float depth) override
        {
            log("glClearDepth({})", depth);
            wrapper->clear_depth(depth);
        }

        void clear_stencil(int32_t stencil) override
        {
            log("glClearStencil({})", stencil);
            wrapper->clear_stencil(stencil);
        }

        void compile_shader(GLuint shader) override
        {
            log("glCompileShader({})", shader);
            wrapper->compile_shader(shader);
        }

        void copy_tex_sub_image_2d(GLenum target, GLint level, GLint xOffset, GLint yOffset,
                                   GLint x, GLint y,
                                   GLsizei width, GLsizei height) override
        {
            log("glCopyTexSubImage2D({}, {}, {}, {}, {}, {}, {}, {})", target, level, xOffset,
                yOffset, x, y, width,
                height);
            wrapper->copy_tex_sub_image_2d(target, level, xOffset, yOffset, x, y, width, height);
        }

        GLuint create_program() override
        {
            log("glCreateProgram()");
            return wrapper->create_program();
        }

        GLuint create_shader(GLenum type) override
        {
            log("glCreateShader({})", type);
            return wrapper->create_shader(type);
        }

        void cull_face(GLenum mode) override
        {
            log("glCullFace({})", mode);
            wrapper->cull_face(mode);
        }

        void delete_buffers(GLsizei n, const GLuint* buffers) override
        {
            log("glDeleteBuffers({}, {})", n, static_cast<const void*>(buffers));
            wrapper->delete_buffers(n, buffers);
        }

        void delete_framebuffers(GLsizei n, const GLuint* framebuffers) override
        {
            log("glDeleteFramebuffers({}, {})", n, static_cast<const void*>(framebuffers));
            wrapper->delete_framebuffers(n, framebuffers);
        }

        void delete_program(GLuint program) override
        {
            log("glDeleteProgram({})", program);
            wrapper->delete_program(program);
        }

        void delete_shader(GLuint shader) override
        {
            log("glDeleteShader({})", shader);
            wrapper->delete_shader(shader);
        }

        void delete_textures(GLsizei n, const GLuint* textures) override
        {
            log("glDeleteTextures({}, {})", n, static_cast<const void*>(textures));
            wrapper->delete_textures(n, textures);
        }

        void delete_vertex_arrays(GLsizei n, const GLuint* arrays) override
        {
            log("glDeleteVertexArrays({}, {})", n, static_cast<const void*>(arrays));
            wrapper->delete_vertex_arrays(n, arrays);
        }

        void disable(GLenum cap) override
        {
            log("glDisable({})", cap);
            wrapper->disable(cap);
        }

        void disable_vertex_attrib_array(GLuint index) override
        {
            log("glDisableVertexAttribArray({})", index);
            wrapper->disable_vertex_attrib_array(index);
        }

        void draw_arrays(GLenum mode, GLint first, GLsizei count) override
        {
            log("glDrawArrays({}, {}, {})", mode, first, count);
            wrapper->draw_arrays(mode, first, count);
        }

        void draw_elements(GLenum mode, GLsizei count, GLenum type, const void* indices) override
        {
            log("glDrawElements({}, {}, {}, {})", mode, count, type,
                static_cast<const void*>(indices));
            wrapper->draw_elements(mode, count, type, indices);
        }

        void enable(GLenum cap) override
        {
            log("glEnable({})", cap);
            wrapper->enable(cap);
        }

        void enable_vertex_attrib_array(GLuint index) override
        {
            log("glEnableVertexAttribArray({})", index);
            wrapper->enable_vertex_attrib_array(index);
        }

        void finish() override
        {
            log("glFinish()");
            wrapper->finish();
        }

        void framebuffer_texture_2d(GLenum target, GLenum attachment, GLenum texTarget,
                                    GLuint texture,
                                    GLint level) override
        {
            log("glFramebufferTexture2D({}, {}, {}, {}, {})", target, attachment, texTarget,
                texture, level);
            wrapper->framebuffer_texture_2d(target, attachment, texTarget, texture, level);
        }

        void gen_buffers(GLsizei n, GLuint* buffers) override
        {
            log("glGenBuffers({}, {})", n, static_cast<const void*>(buffers));
            wrapper->gen_buffers(n, buffers);
        }

        void generate_mipmap(GLenum target) override
        {
            log("glGenerateMipmap({})", target);
            wrapper->generate_mipmap(target);
        }

        void gen_framebuffers(GLsizei n, GLuint* framebuffers) override
        {
            log("glGenFramebuffers({}, {})", n, static_cast<const void*>(framebuffers));
            wrapper->gen_framebuffers(n, framebuffers);
        }

        void gen_textures(GLsizei n, GLuint* textures) override
        {
            log("glGenTextures({}, {})", n, static_cast<const void*>(textures));
            wrapper->gen_textures(n, textures);
        }

        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
            log("glGenVertexArrays({}, {})", n, static_cast<const void*>(arrays));
            wrapper->gen_vertex_arrays(n, arrays);
        }

        GLint get_attrib_location(GLuint program, const GLchar* name) override
        {
            log("glGetAttribLocation({}, \"{}\")", program, std::string_view(name));
            return wrapper->get_attrib_location(program, name);
        }

        void get_buffer_parameter(GLenum target, GLenum pname, GLint* params) override
        {
            log("glGetBufferParameteriv({}, {}, {})", target, pname,
                static_cast<const void*>(params));
            wrapper->get_buffer_parameter(target, pname, params);
        }

        GLenum getError() override
        {
            auto error = wrapper->getError();
            if (error != GL_NO_ERROR)
                log("glGetError() -> {}", error);
            return error;
        }

        void get_integer(GLenum pname, GLint* params) override
        {
            log("glGetIntegerv({}, {})", pname, static_cast<const void*>(params));
            wrapper->get_integer(pname, params);
        }

        void get_program_info_log(GLuint program, GLsizei bufSize, GLsizei* length,
                                  GLchar* infoLog) override
        {
            log("glGetProgramInfoLog({}, {}, {}, {})", program, bufSize,
                static_cast<const void*>(length), static_cast<const void*>(infoLog));
            wrapper->get_program_info_log(program, bufSize, length, infoLog);
        }

        void get_program(GLuint program, GLenum pname, GLint* param) override
        {
            log("glGetProgramiv({}, {}, {})", program, pname, static_cast<const void*>(param));
            wrapper->get_program(program, pname, param);
        }

        void get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei* length,
                                 GLchar* infoLog) override
        {
            log("glGetShaderInfoLog({}, {}, {}, {})", shader, bufSize,
                static_cast<const void*>(length), static_cast<const void*>(infoLog));
            wrapper->get_shader_info_log(shader, bufSize, length, infoLog);
        }

        void get_shader(GLuint shader, GLenum pname, GLint* param) override
        {
            log("glGetShaderiv({}, {}, {})", shader, pname, static_cast<const void*>(param));
            wrapper->get_shader(shader, pname, param);
        }

        const GLubyte* get_string(GLenum pname) override
        {
            log("glGetString({})", pname);
            return wrapper->get_string(pname);
        }

        const GLubyte* get_string_i(GLenum name, GLuint index) override
        {
            log("glGetStringi({}, {})", name, index);
            return wrapper->get_string_i(name, index);
        }

        void get_tex_parameter(GLenum target, GLenum pname, GLfloat* params) override
        {
            log("glGetTexParameterfv({}, {}, {})", target, pname, static_cast<const void*>(params));
            wrapper->get_tex_parameter(target, pname, params);
        }

        void get_tex_parameter(GLenum target, GLenum pname, GLint* params) override
        {
            log("glGetTexParameteriv({}, {}, {})", target, pname, static_cast<const void*>(params));
            wrapper->get_tex_parameter(target, pname, params);
        }

        GLint get_uniform_location(GLuint program, const GLchar* name) override
        {
            log("glGetUniformLocation({}, \"{}\")", program, std::string_view(name));
            return wrapper->get_uniform_location(program, name);
        }

        GLboolean is_buffer(GLuint buffer) override
        {
            log("glIsBuffer(buffer)");
            return wrapper->is_buffer(buffer);
        }

        GLboolean is_enabled(GLenum cap) override
        {
            log("glIsEnabled({})", cap);
            return wrapper->is_enabled(cap);
        }

        void link_program(GLuint program) override
        {
            log("glLinkProgram({})", program);
            wrapper->link_program(program);
        }

        void shader_source(GLuint shader, GLsizei count, const GLchar* const * string,
                           const GLint* length) override
        {
            log("glShaderSource({}, {}, \"{}\")", shader, count,
                std::string_view(*string, *length));
            wrapper->shader_source(shader, count, string, length);
        }

        void tex_image_2d(GLenum target, GLint level, GLint internalformat, GLsizei width,
                          GLsizei height, GLint border,
                          GLenum format, GLenum type, const void* pixels) override
        {
            log("glTexImage2D({}, {}, {}, {}, {}, {}, {}, {}, {})", target, level, internalformat,
                width, height,
                border, format, type, static_cast<const void*>(pixels));
            wrapper->tex_image_2d(target, level, internalformat, width, height, border, format,
                                  type, pixels);
        }

        void tex_parameter_f(GLenum target, GLenum pname, GLfloat param) override
        {
            log("glTexParameterf({}, {}, {})", target, pname, param);
            wrapper->tex_parameter_f(target, pname, param);
        }

        void tex_parameter_i(GLenum target, GLenum pname, GLint param) override
        {
            log("glTexParameteri({}, {}, {})", target, pname, param);
            wrapper->tex_parameter_i(target, pname, param);
        }

        void tex_storage_2d(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
                            GLsizei height) override
        {
            log("glTexStorage2D({}, {}, {}, {}, {})", target, levels, internalformat, width,
                height);
            wrapper->tex_storage_2d(target, levels, internalformat, width, height);
        }

        void tex_sub_image_2d(GLenum target, GLint level, GLint xOffset, GLint yOffset,
                              GLsizei width, GLsizei height,
                              GLenum format, GLenum type, const void* pixels) override
        {
            log("glTexSubImage2D({}, {}, {}, {}, {}, {}, {}, {}, {})", target, level, xOffset,
                yOffset, width, height,
                format, type, static_cast<const void*>(pixels));
            wrapper->tex_sub_image_2d(target, level, xOffset, yOffset, width, height, format, type,
                                      pixels);
        }

        void uniform1_f(GLint location, GLfloat v0) override
        {
            log("glUniform1f({}, {})", location, v0);
            wrapper->uniform1_f(location, v0);
        }

        void uniform1_i(GLint location, GLint v0) override
        {
            log("glUniform1i({}, {})", location, v0);
            wrapper->uniform1_i(location, v0);
        }

        void uniform1(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform1fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform1(location, count, value);
        }

        void uniform1(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform1iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform1(location, count, value);
        }

        void uniform2_f(GLint location, GLfloat v0, GLfloat v1) override
        {
            log("glUniform2f({}, {}, {})", location, v0, v1);
            wrapper->uniform2_f(location, v0, v1);
        }

        void uniform2_i(GLint location, GLint v0, GLint v1) override
        {
            log("glUniform2i({}, {}, {})", location, v0, v1);
            wrapper->uniform2_i(location, v0, v1);
        }

        void uniform2(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform2fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform2(location, count, value);
        }

        void uniform2(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform2iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform2(location, count, value);
        }

        void uniform3_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override
        {
            log("glUniform3f({}, {}, {}, {})", location, v0, v1, v2);
            wrapper->uniform3_f(location, v0, v1, v2);
        }

        void uniform3_i(GLint location, GLint v0, GLint v1, GLint v2) override
        {
            log("glUniform3i({}, {}, {}, {})", location, v0, v1, v2);
            wrapper->uniform3_i(location, v0, v1, v2);
        }

        void uniform3(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform3fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform3(location, count, value);
        }

        void uniform3(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform3iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform3(location, count, value);
        }

        void uniform4_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
        {
            log("glUniform4f({}, {}, {}, {}, {})", location, v0, v1, v2, v3);
            wrapper->uniform4_f(location, v0, v1, v2, v3);
        }

        void uniform4_i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override
        {
            log("glUniform4i({}, {}, {}, {}, {})", location, v0, v1, v2, v3);
            wrapper->uniform4_i(location, v0, v1, v2, v3);
        }

        void uniform4(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform4fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform4(location, count, value);
        }

        void uniform4(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform4iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform4(location, count, value);
        }

        void uniform_matrix2(GLint location, GLsizei count, GLboolean transpose,
                             const GLfloat* value) override
        {
            log("glUniformMatrix2fv({}, {}, {}, {})", location, count, transpose,
                static_cast<const void*>(value));
            wrapper->uniform_matrix2(location, count, transpose, value);
        }

        void uniform_matrix3(GLint location, GLsizei count, GLboolean transpose,
                             const GLfloat* value) override
        {
            log("glUniformMatrix3fv({}, {}, {}, {})", location, count, transpose,
                static_cast<const void*>(value));
            wrapper->uniform_matrix3(location, count, transpose, value);
        }

        void uniform_matrix4(GLint location, GLsizei count, GLboolean transpose,
                             const GLfloat* value) override
        {
            log("glUniformMatrix4fv({}, {}, {}, {})", location, count, transpose,
                static_cast<const void*>(value));
            wrapper->uniform_matrix4(location, count, transpose, value);
        }

        void use_program(GLuint program) override
        {
            log("glUseProgram({})", program);
            wrapper->use_program(program);
        }

        void vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                                   GLsizei stride,
                                   const void* pointer) override
        {
            log("glVertexAttribPointer({}, {}, {}, {}, {}, {})", index, size, type, normalized,
                stride, static_cast<const void*>(pointer));
            wrapper->vertex_attrib_pointer(index, size, type, normalized, stride, pointer);
        }

        void viewport(GLint x, GLint y, GLsizei width, GLsizei height) override
        {
            log("glViewport({}, {}, {}, {})", x, y, width, height);
            wrapper->viewport(x, y, width, height);
        }

        IOglWrapper* wrapper = nullptr;
    };
}
