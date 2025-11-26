//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-26.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "IOpenGlWrapper.h"
#include <GL/glew.h>

namespace Tungsten
{
    class OpenGlWrapper : public IOpenGlWrapper
    {
    public:
        void glActiveTexture(GLenum texture) override
        {
            ::glActiveTexture(texture);
        }

        void glAttachShader(GLuint program, GLuint shader) override
        {
            ::glAttachShader(program, shader);
        }

        void glBindBuffer(GLenum target, GLuint buffer) override
        {
            ::glBindBuffer(target, buffer);
        }

        void glBindFramebuffer(GLenum target, GLuint framebuffer) override
        {
            ::glBindFramebuffer(target, framebuffer);
        }

        void glBindTexture(GLenum target, GLuint texture) override
        {
            ::glBindTexture(target, texture);
        }

        void glBindVertexArray(GLuint array) override
        {
            ::glBindVertexArray(array);
        }

        void glBlendFunc(GLenum sFactor, GLenum dFactor) override
        {
            ::glBlendFunc(sFactor, dFactor);
        }

        void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) override
        {
            ::glBufferData(target, size, data, usage);
        }

        void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) override
        {
            ::glBufferSubData(target, offset, size, data);
        }

        GLenum glCheckFramebufferStatus(GLenum target) override
        {
            ::glCheckFramebufferStatus(target);
        }

        void glClear(GLbitfield mask) override
        {
            ::glClear(mask);
        }

        void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
        {
            ::glClearColor(red, green, blue, alpha);
        }

        void glCompileShader(GLuint shader) override
        {
            ::glCompileShader(shader);
        }

        void glCopyTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLint x, GLint y, GLsizei width, GLsizei height) override
        {
            ::glCopyTexSubImage2D(target, level, xOffset, yOffset, x, y, width, height);
        }

        GLuint glCreateProgram() override
        {
            ::glCreateProgram();
        }

        GLuint glCreateShader(GLenum type) override
        {
            ::glCreateShader(type);
        }

        void glDeleteBuffers(GLsizei n, const GLuint *buffers) override
        {
            ::glDeleteBuffers(n, buffers);
        }

        void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) override
        {
            ::glDeleteFramebuffers(n, framebuffers);
        }

        void glDeleteProgram(GLuint program) override
        {
            ::glDeleteProgram(program);
        }

        void glDeleteShader(GLuint shader) override
        {
            ::glDeleteShader(shader);
        }

        void glDeleteTextures(GLsizei n, const GLuint *textures) override
        {
            ::glDeleteTextures(n, textures);
        }

        void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) override
        {
            ::glDeleteVertexArrays(n, arrays);
        }

        void glDisable(GLenum cap) override
        {
            ::glDisable(cap);
        }

        void glDisableVertexAttribArray(GLuint index) override
        {
            ::glDisableVertexAttribArray(index);
        }

        void glDrawArrays(GLenum mode, GLint first, GLsizei count) override
        {
            ::glDrawArrays(mode, first, count);
        }

        void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) override
        {
            ::glDrawElements(mode, count, type, indices);
        }

        void glEnable(GLenum cap) override
        {
            ::glEnable(cap);
        }

        void glEnableVertexAttribArray(GLuint index) override
        {
            ::glEnableVertexAttribArray(index);
        }

        void glFinish() override
        {
            ::glFinish();
        }

        void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum texTarget, GLuint texture, GLint level) override
        {
            ::glFramebufferTexture2D(target, attachment, texTarget, texture, level);
        }

        void glGenBuffers(GLsizei n, GLuint *buffers) override
        {
            ::glGenBuffers(n, buffers);
        }

        void glGenerateMipmap(GLenum target) override
        {
            ::glGenerateMipmap(target);
        }

        void glGenFramebuffers(GLsizei n, GLuint *framebuffers) override
        {
            ::glGenFramebuffers(n, framebuffers);
        }

        void glGenTextures(GLsizei n, GLuint *textures) override
        {
            ::glGenTextures(n, textures);
        }

        void glGenVertexArrays(GLsizei n, GLuint *arrays) override
        {
            ::glGenVertexArrays(n, arrays);
        }

        GLint glGetAttribLocation(GLuint program, const GLchar *name) override
        {
            ::glGetAttribLocation(program, name);
        }

        void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params) override
        {
            ::glGetBufferParameteriv(target, pname, params);
        }

        GLenum glGetError() override
        {
            return ::glGetError();
        }

        void glGetIntegerv(GLenum pname, GLint *params) override
        {
            ::glGetIntegerv(pname, params);
        }

        void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) override
        {
            ::glGetProgramInfoLog(program, bufSize, length, infoLog);
        }

        void glGetProgramiv(GLuint program, GLenum pname, GLint *param) override
        {
            ::glGetProgramiv(program, pname, param);
        }

        void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) override
        {
            ::glGetShaderInfoLog(shader, bufSize, length, infoLog);
        }

        void glGetShaderiv(GLuint shader, GLenum pname, GLint *param) override
        {
            ::glGetShaderiv(shader, pname, param);
        }

        void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) override
        {
            ::glGetTexParameterfv(target, pname, params);
        }

        void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) override
        {
            ::glGetTexParameteriv(target, pname, params);
        }

        GLint glGetUniformLocation(GLuint program, const GLchar *name) override
        {
            ::glGetUniformLocation(program, name);
        }

        GLboolean glIsBuffer(GLuint buffer) override
        {
            ::glIsBuffer(buffer);
        }

        GLboolean glIsEnabled(GLenum cap) override
        {
            return ::glIsEnabled(cap);
        }

        void glLinkProgram(GLuint program) override
        {
            ::glLinkProgram(program);
        }

        void glShaderSource(GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length) override
        {
            ::glShaderSource(shader, count, string, length);
        }

        void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) override
        {
            ::glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        }

        void glTexParameterf(GLenum target, GLenum pname, GLfloat param) override
        {
            ::glTexParameterf(target, pname, param);
        }

        void glTexParameteri(GLenum target, GLenum pname, GLint param) override
        {
            ::glTexParameteri(target, pname, param);
        }

        void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override
        {
            ::glTexStorage2D(target, levels, internalformat, width, height);
        }

        void glTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) override
        {
            ::glTexSubImage2D(target, level, xOffset, yOffset, width, height, format, type, pixels);
        }

        void glUniform1f(GLint location, GLfloat v0) override
        {
            ::glUniform1f(location, v0);
        }

        void glUniform1i(GLint location, GLint v0) override
        {
            ::glUniform1i(location, v0);
        }

        void glUniform1fv(GLint location, GLsizei count, GLfloat *value) override
        {
            ::glUniform1fv(location, count, value);
        }

        void glUniform1iv(GLint location, GLsizei count, GLint *value) override
        {
            ::glUniform1iv(location, count, value);
        }

        void glUniform2f(GLint location, GLfloat v0, GLfloat v1) override
        {
            ::glUniform2f(location, v0, v1);
        }

        void glUniform2i(GLint location, GLint v0, GLint v1) override
        {
            ::glUniform2i(location, v0, v1);
        }

        void glUniform2fv(GLint location, GLsizei count, GLfloat *value) override
        {
            ::glUniform2fv(location, count, value);
        }

        void glUniform2iv(GLint location, GLsizei count, GLint *value) override
        {
            ::glUniform2iv(location, count, value);
        }

        void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override
        {
            ::glUniform3f(location, v0, v1, v2);
        }

        void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) override
        {
            ::glUniform3i(location, v0, v1, v2);
        }

        void glUniform3fv(GLint location, GLsizei count, GLfloat *value) override
        {
            ::glUniform3fv(location, count, value);
        }

        void glUniform3iv(GLint location, GLsizei count, GLint *value) override
        {
            ::glUniform3iv(location, count, value);
        }

        void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
        {
            ::glUniform4f(location, v0, v1, v2, v3);
        }

        void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override
        {
            ::glUniform4i(location, v0, v1, v2, v3);
        }

        void glUniform4fv(GLint location, GLsizei count, GLfloat *value) override
        {
            ::glUniform4fv(location, count, value);
        }

        void glUniform4iv(GLint location, GLsizei count, GLint *value) override
        {
            ::glUniform4iv(location, count, value);
        }

        void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) override
        {
            ::glUniformMatrix2fv(location, count, transpose, value);
        }

        void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) override
        {
            ::glUniformMatrix3fv(location, count, transpose, value);
        }

        void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) override
        {
            ::glUniformMatrix4fv(location, count, transpose, value);
        }

        void glUseProgram(GLuint program) override
        {
            ::glUseProgram(program);
        }

        void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) override
        {
            ::glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        }

        void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) override
        {
            ::glViewport(x, y, width, height);
        }
    };
}
