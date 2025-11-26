//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>

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
    class IOpenGlWrapper
    {
    public:
        virtual ~IOpenGlWrapper() = default;

        virtual void glActiveTexture(GLenum texture) = 0;

        virtual void glAttachShader(GLuint program, GLuint shader) = 0;

        virtual void glBindBuffer(GLenum target, GLuint buffer) = 0;

        virtual void glBindFramebuffer(GLenum target, GLuint framebuffer) = 0;

        virtual void glBindTexture(GLenum target, GLuint texture) = 0;

        virtual void glBindVertexArray(GLuint array) = 0;

        virtual void glBlendFunc(GLenum sFactor, GLenum dFactor) = 0;

        virtual void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) = 0;

        virtual void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) = 0;

        virtual GLenum glCheckFramebufferStatus(GLenum target) = 0;

        virtual void glClear(GLbitfield mask) = 0;

        virtual void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = 0;

        virtual void glCompileShader(GLuint shader) = 0;

        virtual void glCopyTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLint x, GLint y,
                                         GLsizei width, GLsizei height) = 0;

        virtual GLuint glCreateProgram() = 0;

        virtual GLuint glCreateShader(GLenum type) = 0;

        virtual void glDeleteBuffers(GLsizei n, const GLuint *buffers) = 0;

        virtual void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) = 0;

        virtual void glDeleteProgram(GLuint program) = 0;

        virtual void glDeleteShader(GLuint shader) = 0;

        virtual void glDeleteTextures(GLsizei n, const GLuint *textures) = 0;

        virtual void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) = 0;

        virtual void glDisable(GLenum cap) = 0;

        virtual void glDisableVertexAttribArray(GLuint index) = 0;

        virtual void glDrawArrays(GLenum mode, GLint first, GLsizei count) = 0;

        virtual void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) = 0;

        virtual void glEnable(GLenum cap) = 0;

        virtual void glEnableVertexAttribArray(GLuint index) = 0;

        virtual void glFinish() = 0;

        virtual void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum texTarget, GLuint texture,
                                            GLint level) = 0;

        virtual void glGenBuffers(GLsizei n, GLuint *buffers) = 0;

        virtual void glGenerateMipmap(GLenum target) = 0;

        virtual void glGenFramebuffers(GLsizei n, GLuint *framebuffers) = 0;

        virtual void glGenTextures(GLsizei n, GLuint *textures) = 0;

        virtual void glGenVertexArrays(GLsizei n, GLuint *arrays) = 0;

        virtual GLint glGetAttribLocation(GLuint program, const GLchar *name) = 0;

        virtual void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params) = 0;

        virtual GLenum glGetError() = 0;

        virtual void glGetIntegerv(GLenum pname, GLint *params) = 0;

        virtual void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) = 0;

        virtual void glGetProgramiv(GLuint program, GLenum pname, GLint *param) = 0;

        virtual void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) = 0;

        virtual void glGetShaderiv(GLuint shader, GLenum pname, GLint *param) = 0;

        virtual void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) = 0;

        virtual void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) = 0;

        virtual GLint glGetUniformLocation(GLuint program, const GLchar *name) = 0;

        virtual GLboolean glIsBuffer(GLuint buffer) = 0;

        virtual GLboolean glIsEnabled(GLenum cap) = 0;

        virtual void glLinkProgram(GLuint program) = 0;

        virtual void glShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) = 0;

        virtual void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                                  GLint border, GLenum format, GLenum type, const void *pixels) = 0;

        virtual void glTexParameterf(GLenum target, GLenum pname, GLfloat param) = 0;

        virtual void glTexParameteri(GLenum target, GLenum pname, GLint param) = 0;

        virtual void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
                                    GLsizei height) = 0;

        virtual void glTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLsizei width,
                                     GLsizei height, GLenum format, GLenum type, const void *pixels) = 0;

        virtual void glUniform1f(GLint location, GLfloat v0) = 0;

        virtual void glUniform1i(GLint location, GLint v0) = 0;

        virtual void glUniform1fv(GLint location, GLsizei count, GLfloat *value) = 0;

        virtual void glUniform1iv(GLint location, GLsizei count, GLint *value) = 0;

        virtual void glUniform2f(GLint location, GLfloat v0, GLfloat v1) = 0;

        virtual void glUniform2i(GLint location, GLint v0, GLint v1) = 0;

        virtual void glUniform2fv(GLint location, GLsizei count, GLfloat *value) = 0;

        virtual void glUniform2iv(GLint location, GLsizei count, GLint *value) = 0;

        virtual void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = 0;

        virtual void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) = 0;

        virtual void glUniform3fv(GLint location, GLsizei count, GLfloat *value) = 0;

        virtual void glUniform3iv(GLint location, GLsizei count, GLint *value) = 0;

        virtual void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = 0;

        virtual void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) = 0;

        virtual void glUniform4fv(GLint location, GLsizei count, GLfloat *value) = 0;

        virtual void glUniform4iv(GLint location, GLsizei count, GLint *value) = 0;

        virtual void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;

        virtual void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;

        virtual void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;

        virtual void glUseProgram(GLuint program) = 0;

        virtual void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                           const void *pointer) = 0;

        virtual void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) = 0;
    };
}
