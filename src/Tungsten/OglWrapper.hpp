//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/IOglWrapper.hpp"
#include <GL/glew.h>

namespace Tungsten
{
    class OglWrapper final : public IOglWrapper
    {
    public:
        void active_texture(GLenum texture) override
        {
            glActiveTexture(texture);
        }

        void attach_shader(GLuint program, GLuint shader) override
        {
            glAttachShader(program, shader);
        }

        void bind_buffer(GLenum target, GLuint buffer) override
        {
            glBindBuffer(target, buffer);
        }

        void bind_framebuffer(GLenum target, GLuint framebuffer) override
        {
            glBindFramebuffer(target, framebuffer);
        }

        void bind_texture(GLenum target, GLuint texture) override
        {
            glBindTexture(target, texture);
        }

        void bind_vertex_array(GLuint array) override
        {
            glBindVertexArray(array);
        }

        void blend_func(GLenum sFactor, GLenum dFactor) override
        {
            glBlendFunc(sFactor, dFactor);
        }

        void buffer_data(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override
        {
            glBufferData(target, size, data, usage);
        }

        void buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) override
        {
            glBufferSubData(target, offset, size, data);
        }

        GLenum check_framebuffer_status(GLenum target) override
        {
            return glCheckFramebufferStatus(target);
        }

        void clear(GLbitfield mask) override
        {
            glClear(mask);
        }

        void clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
        {
            glClearColor(red, green, blue, alpha);
        }

        void clear_depth(float depth) override
        {
            glClearDepth(depth);
        }

        void clear_stencil(int32_t stencil) override
        {
            glClearStencil(stencil);
        }

        void compile_shader(GLuint shader) override
        {
            glCompileShader(shader);
        }

        void copy_tex_sub_image_2d(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLint x, GLint y,
                               GLsizei width, GLsizei height) override
        {
            glCopyTexSubImage2D(target, level, xOffset, yOffset, x, y, width, height);
        }

        GLuint create_program() override
        {
            return glCreateProgram();
        }

        GLuint create_shader(GLenum type) override
        {
            return glCreateShader(type);
        }

        void cull_face(GLenum mode) override
        {
            glCullFace(mode);
        }

        void delete_buffers(GLsizei n, const GLuint* buffers) override
        {
            glDeleteBuffers(n, buffers);
        }

        void delete_framebuffers(GLsizei n, const GLuint* framebuffers) override
        {
            glDeleteFramebuffers(n, framebuffers);
        }

        void delete_program(GLuint program) override
        {
            glDeleteProgram(program);
        }

        void delete_shader(GLuint shader) override
        {
            glDeleteShader(shader);
        }

        void delete_textures(GLsizei n, const GLuint* textures) override
        {
            glDeleteTextures(n, textures);
        }

        void delete_vertex_arrays(GLsizei n, const GLuint* arrays) override
        {
            glDeleteVertexArrays(n, arrays);
        }

        void disable(GLenum cap) override
        {
            glDisable(cap);
        }

        void disable_vertex_attrib_array(GLuint index) override
        {
            glDisableVertexAttribArray(index);
        }

        void draw_arrays(GLenum mode, GLint first, GLsizei count) override
        {
            glDrawArrays(mode, first, count);
        }

        void draw_elements(GLenum mode, GLsizei count, GLenum type, const void* indices) override
        {
            glDrawElements(mode, count, type, indices);
        }

        void enable(GLenum cap) override
        {
            glEnable(cap);
        }

        void enable_vertex_attrib_array(GLuint index) override
        {
            glEnableVertexAttribArray(index);
        }

        void finish() override
        {
            glFinish();
        }

        void framebuffer_texture_2d(GLenum target, GLenum attachment, GLenum texTarget, GLuint texture,
                                  GLint level) override
        {
            glFramebufferTexture2D(target, attachment, texTarget, texture, level);
        }

        void gen_buffers(GLsizei n, GLuint* buffers) override
        {
            glGenBuffers(n, buffers);
        }

        void generate_mipmap(GLenum target) override
        {
            glGenerateMipmap(target);
        }

        void gen_framebuffers(GLsizei n, GLuint* framebuffers) override
        {
            glGenFramebuffers(n, framebuffers);
        }

        void gen_textures(GLsizei n, GLuint* textures) override
        {
            glGenTextures(n, textures);
        }

        void gen_vertex_arrays(GLsizei n, GLuint* arrays) override
        {
            glGenVertexArrays(n, arrays);
        }

        GLint get_attrib_location(GLuint program, const GLchar* name) override
        {
            return glGetAttribLocation(program, name);
        }

        void get_buffer_parameter(GLenum target, GLenum pname, GLint* params) override
        {
            glGetBufferParameteriv(target, pname, params);
        }

        GLenum getError() override
        {
            return glGetError();
        }

        void get_integer(GLenum pname, GLint* params) override
        {
            glGetIntegerv(pname, params);
        }

        void get_program_info_log(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) override
        {
            glGetProgramInfoLog(program, bufSize, length, infoLog);
        }

        void get_program(GLuint program, GLenum pname, GLint* param) override
        {
            glGetProgramiv(program, pname, param);
        }

        void get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) override
        {
            glGetShaderInfoLog(shader, bufSize, length, infoLog);
        }

        void get_shader(GLuint shader, GLenum pname, GLint* param) override
        {
            glGetShaderiv(shader, pname, param);
        }

        const GLubyte* get_string(GLenum pname) override
        {
            return glGetString(pname);
        }

        const GLubyte* get_string_i(GLenum name, GLuint index) override
        {
            return glGetStringi(name, index);
        }

        void get_tex_parameter(GLenum target, GLenum pname, GLfloat* params) override
        {
            glGetTexParameterfv(target, pname, params);
        }

        void get_tex_parameter(GLenum target, GLenum pname, GLint* params) override
        {
            glGetTexParameteriv(target, pname, params);
        }

        GLint get_uniform_location(GLuint program, const GLchar* name) override
        {
            return glGetUniformLocation(program, name);
        }

        GLboolean is_buffer(GLuint buffer) override
        {
            return glIsBuffer(buffer);
        }

        GLboolean is_enabled(GLenum cap) override
        {
            return glIsEnabled(cap);
        }

        void link_program(GLuint program) override
        {
            glLinkProgram(program);
        }

        void shader_source(GLuint shader, GLsizei count, const GLchar* const * string, const GLint* length) override
        {
            glShaderSource(shader, count, string, length);
        }

        void tex_image_2d(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
                        GLenum format, GLenum type, const void* pixels) override
        {
            glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        }

        void tex_parameter_f(GLenum target, GLenum pname, GLfloat param) override
        {
            glTexParameterf(target, pname, param);
        }

        void tex_parameter_i(GLenum target, GLenum pname, GLint param) override
        {
            glTexParameteri(target, pname, param);
        }

        void tex_storage_2d(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override
        {
            glTexStorage2D(target, levels, internalformat, width, height);
        }

        void tex_sub_image_2d(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const void* pixels) override
        {
            glTexSubImage2D(target, level, xOffset, yOffset, width, height, format, type, pixels);
        }

        void uniform1_f(GLint location, GLfloat v0) override
        {
            glUniform1f(location, v0);
        }

        void uniform1_i(GLint location, GLint v0) override
        {
            glUniform1i(location, v0);
        }

        void uniform1(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform1fv(location, count, value);
        }

        void uniform1(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform1iv(location, count, value);
        }

        void uniform2_f(GLint location, GLfloat v0, GLfloat v1) override
        {
            glUniform2f(location, v0, v1);
        }

        void uniform2_i(GLint location, GLint v0, GLint v1) override
        {
            glUniform2i(location, v0, v1);
        }

        void uniform2(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform2fv(location, count, value);
        }

        void uniform2(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform2iv(location, count, value);
        }

        void uniform3_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override
        {
            glUniform3f(location, v0, v1, v2);
        }

        void uniform3_i(GLint location, GLint v0, GLint v1, GLint v2) override
        {
            glUniform3i(location, v0, v1, v2);
        }

        void uniform3(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform3fv(location, count, value);
        }

        void uniform3(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform3iv(location, count, value);
        }

        void uniform4_f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
        {
            glUniform4f(location, v0, v1, v2, v3);
        }

        void uniform4_i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override
        {
            glUniform4i(location, v0, v1, v2, v3);
        }

        void uniform4(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform4fv(location, count, value);
        }

        void uniform4(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform4iv(location, count, value);
        }

        void uniform_matrix2(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            glUniformMatrix2fv(location, count, transpose, value);
        }

        void uniform_matrix3(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            glUniformMatrix3fv(location, count, transpose, value);
        }

        void uniform_matrix4(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            glUniformMatrix4fv(location, count, transpose, value);
        }

        void use_program(GLuint program) override
        {
            glUseProgram(program);
        }

        void vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                 const void* pointer) override
        {
            glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        }

        void viewport(GLint x, GLint y, GLsizei width, GLsizei height) override
        {
            glViewport(x, y, width, height);
        }
    };
}
