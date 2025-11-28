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
        void activeTexture(GLenum texture) override
        {
            glActiveTexture(texture);
        }

        void attachShader(GLuint program, GLuint shader) override
        {
            glAttachShader(program, shader);
        }

        void bindBuffer(GLenum target, GLuint buffer) override
        {
            glBindBuffer(target, buffer);
        }

        void bindFramebuffer(GLenum target, GLuint framebuffer) override
        {
            glBindFramebuffer(target, framebuffer);
        }

        void bindTexture(GLenum target, GLuint texture) override
        {
            glBindTexture(target, texture);
        }

        void bindVertexArray(GLuint array) override
        {
            glBindVertexArray(array);
        }

        void blendFunc(GLenum sFactor, GLenum dFactor) override
        {
            glBlendFunc(sFactor, dFactor);
        }

        void bufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override
        {
            glBufferData(target, size, data, usage);
        }

        void bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) override
        {
            glBufferSubData(target, offset, size, data);
        }

        GLenum checkFramebufferStatus(GLenum target) override
        {
            return glCheckFramebufferStatus(target);
        }

        void clear(GLbitfield mask) override
        {
            glClear(mask);
        }

        void clearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
        {
            glClearColor(red, green, blue, alpha);
        }

        void compileShader(GLuint shader) override
        {
            glCompileShader(shader);
        }

        void copyTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLint x, GLint y,
                               GLsizei width, GLsizei height) override
        {
            glCopyTexSubImage2D(target, level, xOffset, yOffset, x, y, width, height);
        }

        GLuint createProgram() override
        {
            return glCreateProgram();
        }

        GLuint createShader(GLenum type) override
        {
            return glCreateShader(type);
        }

        void deleteBuffers(GLsizei n, const GLuint* buffers) override
        {
            glDeleteBuffers(n, buffers);
        }

        void deleteFramebuffers(GLsizei n, const GLuint* framebuffers) override
        {
            glDeleteFramebuffers(n, framebuffers);
        }

        void deleteProgram(GLuint program) override
        {
            glDeleteProgram(program);
        }

        void deleteShader(GLuint shader) override
        {
            glDeleteShader(shader);
        }

        void deleteTextures(GLsizei n, const GLuint* textures) override
        {
            glDeleteTextures(n, textures);
        }

        void deleteVertexArrays(GLsizei n, const GLuint* arrays) override
        {
            glDeleteVertexArrays(n, arrays);
        }

        void disable(GLenum cap) override
        {
            glDisable(cap);
        }

        void disableVertexAttribArray(GLuint index) override
        {
            glDisableVertexAttribArray(index);
        }

        void drawArrays(GLenum mode, GLint first, GLsizei count) override
        {
            glDrawArrays(mode, first, count);
        }

        void drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) override
        {
            glDrawElements(mode, count, type, indices);
        }

        void enable(GLenum cap) override
        {
            glEnable(cap);
        }

        void enableVertexAttribArray(GLuint index) override
        {
            glEnableVertexAttribArray(index);
        }

        void finish() override
        {
            glFinish();
        }

        void framebufferTexture2D(GLenum target, GLenum attachment, GLenum texTarget, GLuint texture,
                                  GLint level) override
        {
            glFramebufferTexture2D(target, attachment, texTarget, texture, level);
        }

        void genBuffers(GLsizei n, GLuint* buffers) override
        {
            glGenBuffers(n, buffers);
        }

        void generateMipmap(GLenum target) override
        {
            glGenerateMipmap(target);
        }

        void genFramebuffers(GLsizei n, GLuint* framebuffers) override
        {
            glGenFramebuffers(n, framebuffers);
        }

        void genTextures(GLsizei n, GLuint* textures) override
        {
            glGenTextures(n, textures);
        }

        void genVertexArrays(GLsizei n, GLuint* arrays) override
        {
            glGenVertexArrays(n, arrays);
        }

        GLint getAttribLocation(GLuint program, const GLchar* name) override
        {
            return glGetAttribLocation(program, name);
        }

        void getBufferParameteriv(GLenum target, GLenum pname, GLint* params) override
        {
            glGetBufferParameteriv(target, pname, params);
        }

        GLenum getError() override
        {
            return glGetError();
        }

        void getIntegerv(GLenum pname, GLint* params) override
        {
            glGetIntegerv(pname, params);
        }

        void getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) override
        {
            glGetProgramInfoLog(program, bufSize, length, infoLog);
        }

        void getProgramiv(GLuint program, GLenum pname, GLint* param) override
        {
            glGetProgramiv(program, pname, param);
        }

        void getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) override
        {
            glGetShaderInfoLog(shader, bufSize, length, infoLog);
        }

        void getShaderiv(GLuint shader, GLenum pname, GLint* param) override
        {
            glGetShaderiv(shader, pname, param);
        }

        void getTexParameterfv(GLenum target, GLenum pname, GLfloat* params) override
        {
            glGetTexParameterfv(target, pname, params);
        }

        void getTexParameteriv(GLenum target, GLenum pname, GLint* params) override
        {
            glGetTexParameteriv(target, pname, params);
        }

        GLint getUniformLocation(GLuint program, const GLchar* name) override
        {
            return glGetUniformLocation(program, name);
        }

        GLboolean isBuffer(GLuint buffer) override
        {
            return glIsBuffer(buffer);
        }

        GLboolean isEnabled(GLenum cap) override
        {
            return glIsEnabled(cap);
        }

        void linkProgram(GLuint program) override
        {
            glLinkProgram(program);
        }

        void shaderSource(GLuint shader, GLsizei count, const GLchar* const * string, const GLint* length) override
        {
            glShaderSource(shader, count, string, length);
        }

        void texImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
                        GLenum format, GLenum type, const void* pixels) override
        {
            glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        }

        void texParameterf(GLenum target, GLenum pname, GLfloat param) override
        {
            glTexParameterf(target, pname, param);
        }

        void texParameteri(GLenum target, GLenum pname, GLint param) override
        {
            glTexParameteri(target, pname, param);
        }

        void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override
        {
            glTexStorage2D(target, levels, internalformat, width, height);
        }

        void texSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const void* pixels) override
        {
            glTexSubImage2D(target, level, xOffset, yOffset, width, height, format, type, pixels);
        }

        void uniform1f(GLint location, GLfloat v0) override
        {
            glUniform1f(location, v0);
        }

        void uniform1i(GLint location, GLint v0) override
        {
            glUniform1i(location, v0);
        }

        void uniform1fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform1fv(location, count, value);
        }

        void uniform1iv(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform1iv(location, count, value);
        }

        void uniform2f(GLint location, GLfloat v0, GLfloat v1) override
        {
            glUniform2f(location, v0, v1);
        }

        void uniform2i(GLint location, GLint v0, GLint v1) override
        {
            glUniform2i(location, v0, v1);
        }

        void uniform2fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform2fv(location, count, value);
        }

        void uniform2iv(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform2iv(location, count, value);
        }

        void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override
        {
            glUniform3f(location, v0, v1, v2);
        }

        void uniform3i(GLint location, GLint v0, GLint v1, GLint v2) override
        {
            glUniform3i(location, v0, v1, v2);
        }

        void uniform3fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform3fv(location, count, value);
        }

        void uniform3iv(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform3iv(location, count, value);
        }

        void uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
        {
            glUniform4f(location, v0, v1, v2, v3);
        }

        void uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override
        {
            glUniform4i(location, v0, v1, v2, v3);
        }

        void uniform4fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            glUniform4fv(location, count, value);
        }

        void uniform4iv(GLint location, GLsizei count, const GLint* value) override
        {
            glUniform4iv(location, count, value);
        }

        void uniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            glUniformMatrix2fv(location, count, transpose, value);
        }

        void uniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            glUniformMatrix3fv(location, count, transpose, value);
        }

        void uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            glUniformMatrix4fv(location, count, transpose, value);
        }

        void useProgram(GLuint program) override
        {
            glUseProgram(program);
        }

        void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
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
