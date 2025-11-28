//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
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
using GLboolean = unsigned char;
using GLfloat = float;

namespace Tungsten
{
    class IOglWrapper
    {
    public:
        virtual ~IOglWrapper() = default;

        virtual void activeTexture(GLenum texture) = 0;

        virtual void attachShader(GLuint program, GLuint shader) = 0;

        virtual void bindBuffer(GLenum target, GLuint buffer) = 0;

        virtual void bindFramebuffer(GLenum target, GLuint framebuffer) = 0;

        virtual void bindTexture(GLenum target, GLuint texture) = 0;

        virtual void bindVertexArray(GLuint array) = 0;

        virtual void blendFunc(GLenum sFactor, GLenum dFactor) = 0;

        virtual void bufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) = 0;

        virtual void bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) = 0;

        virtual GLenum checkFramebufferStatus(GLenum target) = 0;

        virtual void clear(GLbitfield mask) = 0;

        virtual void clearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = 0;

        virtual void compileShader(GLuint shader) = 0;

        virtual void copyTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLint x, GLint y,
                                       GLsizei width, GLsizei height) = 0;

        virtual GLuint createProgram() = 0;

        virtual GLuint createShader(GLenum type) = 0;

        virtual void deleteBuffers(GLsizei n, const GLuint* buffers) = 0;

        virtual void deleteFramebuffers(GLsizei n, const GLuint* framebuffers) = 0;

        virtual void deleteProgram(GLuint program) = 0;

        virtual void deleteShader(GLuint shader) = 0;

        virtual void deleteTextures(GLsizei n, const GLuint* textures) = 0;

        virtual void deleteVertexArrays(GLsizei n, const GLuint* arrays) = 0;

        virtual void disable(GLenum cap) = 0;

        virtual void disableVertexAttribArray(GLuint index) = 0;

        virtual void drawArrays(GLenum mode, GLint first, GLsizei count) = 0;

        virtual void drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) = 0;

        virtual void enable(GLenum cap) = 0;

        virtual void enableVertexAttribArray(GLuint index) = 0;

        virtual void finish() = 0;

        virtual void framebufferTexture2D(GLenum target, GLenum attachment, GLenum texTarget, GLuint texture,
                                          GLint level) = 0;

        virtual void genBuffers(GLsizei n, GLuint* buffers) = 0;

        virtual void generateMipmap(GLenum target) = 0;

        virtual void genFramebuffers(GLsizei n, GLuint* framebuffers) = 0;

        virtual void genTextures(GLsizei n, GLuint* textures) = 0;

        virtual void genVertexArrays(GLsizei n, GLuint* arrays) = 0;

        virtual GLint getAttribLocation(GLuint program, const GLchar* name) = 0;

        virtual void getBufferParameteriv(GLenum target, GLenum pname, GLint* params) = 0;

        virtual GLenum getError() = 0;

        virtual void getIntegerv(GLenum pname, GLint* params) = 0;

        virtual void getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) = 0;

        virtual void getProgramiv(GLuint program, GLenum pname, GLint* param) = 0;

        virtual void getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) = 0;

        virtual void getShaderiv(GLuint shader, GLenum pname, GLint* param) = 0;

        virtual void getTexParameterfv(GLenum target, GLenum pname, GLfloat* params) = 0;

        virtual void getTexParameteriv(GLenum target, GLenum pname, GLint* params) = 0;

        virtual GLint getUniformLocation(GLuint program, const GLchar* name) = 0;

        virtual GLboolean isBuffer(GLuint buffer) = 0;

        virtual GLboolean isEnabled(GLenum cap) = 0;

        virtual void linkProgram(GLuint program) = 0;

        virtual void shaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) = 0;

        virtual void texImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                                GLint border, GLenum format, GLenum type, const void* pixels) = 0;

        virtual void texParameterf(GLenum target, GLenum pname, GLfloat param) = 0;

        virtual void texParameteri(GLenum target, GLenum pname, GLint param) = 0;

        virtual void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
                                  GLsizei height) = 0;

        virtual void texSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLsizei width,
                                   GLsizei height, GLenum format, GLenum type, const void* pixels) = 0;

        virtual void uniform1f(GLint location, GLfloat v0) = 0;

        virtual void uniform1i(GLint location, GLint v0) = 0;

        virtual void uniform1fv(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform1iv(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform2f(GLint location, GLfloat v0, GLfloat v1) = 0;

        virtual void uniform2i(GLint location, GLint v0, GLint v1) = 0;

        virtual void uniform2fv(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform2iv(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = 0;

        virtual void uniform3i(GLint location, GLint v0, GLint v1, GLint v2) = 0;

        virtual void uniform3fv(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform3iv(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = 0;

        virtual void uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) = 0;

        virtual void uniform4fv(GLint location, GLsizei count, const GLfloat* value) = 0;

        virtual void uniform4iv(GLint location, GLsizei count, const GLint* value) = 0;

        virtual void uniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;

        virtual void uniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;

        virtual void uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;

        virtual void useProgram(GLuint program) = 0;

        virtual void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
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
