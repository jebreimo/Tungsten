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
        template<class... Args>
        void log(std::format_string<Args...> fmt, Args&&... args)
        {
            auto str = std::vformat(fmt.get(), std::make_format_args(args...));
            std::clog << str << std::endl;
        }

        void activeTexture(GLenum texture) override
        {
            log("glActiveTexture({})", texture);
            wrapper->activeTexture(texture);
        }

        void attachShader(GLuint program, GLuint shader) override
        {
            log("glAttachShader({}, {})", program, shader);
            wrapper->attachShader(program, shader);
        }

        void bindBuffer(GLenum target, GLuint buffer) override
        {
            log("glBindBuffer({}, {})", target, buffer);
            wrapper->bindBuffer(target, buffer);
        }

        void bindFramebuffer(GLenum target, GLuint framebuffer) override
        {
            log("glBindFramebuffer({}, {})", target, framebuffer);
            wrapper->bindFramebuffer(target, framebuffer);
        }

        void bindTexture(GLenum target, GLuint texture) override
        {
            log("glBindTexture({}, {})", target, texture);
            wrapper->bindTexture(target, texture);
        }

        void bindVertexArray(GLuint array) override
        {
            log("glBindVertexArray({})", array);
            wrapper->bindVertexArray(array);
        }

        void blendFunc(GLenum sFactor, GLenum dFactor) override
        {
            log("glBlendFunc({}, {})", sFactor, dFactor);
            wrapper->blendFunc(sFactor, dFactor);
        }

        void bufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override
        {
            log("glBufferData({}, {}, {}, {})", target, size, static_cast<const void*>(data), usage);
            wrapper->bufferData(target, size, data, usage);
        }

        void bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) override
        {
            log("glBufferSubData({}, {}, {}, {})", target, offset, size, static_cast<const void*>(data));
            wrapper->bufferSubData(target, offset, size, data);
        }

        GLenum checkFramebufferStatus(GLenum target) override
        {
            log("glCheckFramebufferStatus({})", target);
            return glCheckFramebufferStatus(target);
        }

        void clear(GLbitfield mask) override
        {
            log("glClear({})", mask);
            wrapper->clear(mask);
        }

        void clearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
        {
            log("glClearColor({}, {}, {}, {})", red, green, blue, alpha);
            wrapper->clearColor(red, green, blue, alpha);
        }

        void compileShader(GLuint shader) override
        {
            log("glCompileShader({})", shader);
            wrapper->compileShader(shader);
        }

        void copyTexSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLint x, GLint y,
                               GLsizei width, GLsizei height) override
        {
            log("glCopyTexSubImage2D({}, {}, {}, {}, {}, {}, {}, {})", target, level, xOffset, yOffset, x, y, width,
                height);
            wrapper->copyTexSubImage2D(target, level, xOffset, yOffset, x, y, width, height);
        }

        GLuint createProgram() override
        {
            log("glCreateProgram()");
            return glCreateProgram();
        }

        GLuint createShader(GLenum type) override
        {
            log("glCreateShader({})", type);
            return glCreateShader(type);
        }

        void deleteBuffers(GLsizei n, const GLuint* buffers) override
        {
            log("glDeleteBuffers({}, {})", n, static_cast<const void*>(buffers));
            wrapper->deleteBuffers(n, buffers);
        }

        void deleteFramebuffers(GLsizei n, const GLuint* framebuffers) override
        {
            log("glDeleteFramebuffers({}, {})", n, static_cast<const void*>(framebuffers));
            wrapper->deleteFramebuffers(n, framebuffers);
        }

        void deleteProgram(GLuint program) override
        {
            log("glDeleteProgram({})", program);
            wrapper->deleteProgram(program);
        }

        void deleteShader(GLuint shader) override
        {
            log("glDeleteShader({})", shader);
            wrapper->deleteShader(shader);
        }

        void deleteTextures(GLsizei n, const GLuint* textures) override
        {
            log("glDeleteTextures({}, {})", n, static_cast<const void*>(textures));
            wrapper->deleteTextures(n, textures);
        }

        void deleteVertexArrays(GLsizei n, const GLuint* arrays) override
        {
            log("glDeleteVertexArrays({}, {})", n, static_cast<const void*>(arrays));
            wrapper->deleteVertexArrays(n, arrays);
        }

        void disable(GLenum cap) override
        {
            log("glDisable({})", cap);
            wrapper->disable(cap);
        }

        void disableVertexAttribArray(GLuint index) override
        {
            log("glDisableVertexAttribArray({})", index);
            wrapper->disableVertexAttribArray(index);
        }

        void drawArrays(GLenum mode, GLint first, GLsizei count) override
        {
            log("glDrawArrays({}, {}, {})", mode, first, count);
            wrapper->drawArrays(mode, first, count);
        }

        void drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) override
        {
            log("glDrawElements({}, {}, {}, {})", mode, count, type, static_cast<const void*>(indices));
            wrapper->drawElements(mode, count, type, indices);
        }

        void enable(GLenum cap) override
        {
            log("glEnable({})", cap);
            wrapper->enable(cap);
        }

        void enableVertexAttribArray(GLuint index) override
        {
            log("glEnableVertexAttribArray({})", index);
            wrapper->enableVertexAttribArray(index);
        }

        void finish() override
        {
            log("glFinish()");
            wrapper->finish();
        }

        void framebufferTexture2D(GLenum target, GLenum attachment, GLenum texTarget, GLuint texture,
                                  GLint level) override
        {
            log("glFramebufferTexture2D({}, {}, {}, {}, {})", target, attachment, texTarget, texture, level);
            wrapper->framebufferTexture2D(target, attachment, texTarget, texture, level);
        }

        void genBuffers(GLsizei n, GLuint* buffers) override
        {
            log("glGenBuffers({}, {})", n, static_cast<const void*>(buffers));
            wrapper->genBuffers(n, buffers);
        }

        void generateMipmap(GLenum target) override
        {
            log("glGenerateMipmap({})", target);
            wrapper->generateMipmap(target);
        }

        void genFramebuffers(GLsizei n, GLuint* framebuffers) override
        {
            log("glGenFramebuffers({}, {})", n, static_cast<const void*>(framebuffers));
            wrapper->genFramebuffers(n, framebuffers);
        }

        void genTextures(GLsizei n, GLuint* textures) override
        {
            log("glGenTextures({}, {})", n, static_cast<const void*>(textures));
            wrapper->genTextures(n, textures);
        }

        void genVertexArrays(GLsizei n, GLuint* arrays) override
        {
            log("glGenVertexArrays({}, {})", n, static_cast<const void*>(arrays));
            wrapper->genVertexArrays(n, arrays);
        }

        GLint getAttribLocation(GLuint program, const GLchar* name) override
        {
            log("glGetAttribLocation({}, {})", program, std::string_view(name));
            return glGetAttribLocation(program, name);
        }

        void getBufferParameteriv(GLenum target, GLenum pname, GLint* params) override
        {
            log("glGetBufferParameteriv({}, {}, {})", target, pname, static_cast<const void*>(params));
            wrapper->getBufferParameteriv(target, pname, params);
        }

        GLenum getError() override
        {
            auto error = glGetError();
            if (error != GL_NO_ERROR)
                log("glGetError() -> {}", error);
            return error;
        }

        void getIntegerv(GLenum pname, GLint* params) override
        {
            log("glGetIntegerv({}, {})", pname, static_cast<const void*>(params));
            wrapper->getIntegerv(pname, params);
        }

        void getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) override
        {
            log("glGetProgramInfoLog({}, {}, {}, {})", program, bufSize, static_cast<const void*>(length), static_cast<const void*>(infoLog));
            wrapper->getProgramInfoLog(program, bufSize, length, infoLog);
        }

        void getProgramiv(GLuint program, GLenum pname, GLint* param) override
        {
            log("glGetProgramiv({}, {}, {})", program, pname, static_cast<const void*>(param));
            wrapper->getProgramiv(program, pname, param);
        }

        void getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) override
        {
            log("glGetShaderInfoLog({}, {}, {}, {})", shader, bufSize, static_cast<const void*>(length), static_cast<const void*>(infoLog));
            wrapper->getShaderInfoLog(shader, bufSize, length, infoLog);
        }

        void getShaderiv(GLuint shader, GLenum pname, GLint* param) override
        {
            log("glGetShaderiv({}, {}, {})", shader, pname, static_cast<const void*>(param));
            wrapper->getShaderiv(shader, pname, param);
        }

        void getTexParameterfv(GLenum target, GLenum pname, GLfloat* params) override
        {
            log("glGetTexParameterfv({}, {}, {})", target, pname, static_cast<const void*>(params));
            wrapper->getTexParameterfv(target, pname, params);
        }

        void getTexParameteriv(GLenum target, GLenum pname, GLint* params) override
        {
            log("glGetTexParameteriv({}, {}, {})", target, pname, static_cast<const void*>(params));
            wrapper->getTexParameteriv(target, pname, params);
        }

        GLint getUniformLocation(GLuint program, const GLchar* name) override
        {
            log("glGetUniformLocation({}, {})", program, std::string_view(name));
            return glGetUniformLocation(program, name);
        }

        GLboolean isBuffer(GLuint buffer) override
        {
            log("glIsBuffer(buffer)");
            return glIsBuffer(buffer);
        }

        GLboolean isEnabled(GLenum cap) override
        {
            log("glIsEnabled(cap)");
            return glIsEnabled(cap);
        }

        void linkProgram(GLuint program) override
        {
            log("glLinkProgram({})", program);
            wrapper->linkProgram(program);
        }

        void shaderSource(GLuint shader, GLsizei count, const GLchar* const * string, const GLint* length) override
        {
            log("glShaderSource({}, {}, \"{}\")", shader, count, std::string_view(*string, *length));
            wrapper->shaderSource(shader, count, string, length);
        }

        void texImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
                        GLenum format, GLenum type, const void* pixels) override
        {
            log("glTexImage2D({}, {}, {}, {}, {}, {}, {}, {}, {})", target, level, internalformat, width, height,
                border, format, type, static_cast<const void*>(pixels));
            wrapper->texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        }

        void texParameterf(GLenum target, GLenum pname, GLfloat param) override
        {
            log("glTexParameterf({}, {}, {})", target, pname, param);
            wrapper->texParameterf(target, pname, param);
        }

        void texParameteri(GLenum target, GLenum pname, GLint param) override
        {
            log("glTexParameteri({}, {}, {})", target, pname, param);
            wrapper->texParameteri(target, pname, param);
        }

        void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override
        {
            log("glTexStorage2D({}, {}, {}, {}, {})", target, levels, internalformat, width, height);
            wrapper->texStorage2D(target, levels, internalformat, width, height);
        }

        void texSubImage2D(GLenum target, GLint level, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const void* pixels) override
        {
            log("glTexSubImage2D({}, {}, {}, {}, {}, {}, {}, {}, {})", target, level, xOffset, yOffset, width, height,
                format, type, static_cast<const void*>(pixels));
            wrapper->texSubImage2D(target, level, xOffset, yOffset, width, height, format, type, pixels);
        }

        void uniform1f(GLint location, GLfloat v0) override
        {
            log("glUniform1f({}, {})", location, v0);
            wrapper->uniform1f(location, v0);
        }

        void uniform1i(GLint location, GLint v0) override
        {
            log("glUniform1i({}, {})", location, v0);
            wrapper->uniform1i(location, v0);
        }

        void uniform1fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform1fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform1fv(location, count, value);
        }

        void uniform1iv(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform1iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform1iv(location, count, value);
        }

        void uniform2f(GLint location, GLfloat v0, GLfloat v1) override
        {
            log("glUniform2f({}, {}, {})", location, v0, v1);
            wrapper->uniform2f(location, v0, v1);
        }

        void uniform2i(GLint location, GLint v0, GLint v1) override
        {
            log("glUniform2i({}, {}, {})", location, v0, v1);
            wrapper->uniform2i(location, v0, v1);
        }

        void uniform2fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform2fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform2fv(location, count, value);
        }

        void uniform2iv(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform2iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform2iv(location, count, value);
        }

        void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) override
        {
            log("glUniform3f({}, {}, {}, {})", location, v0, v1, v2);
            wrapper->uniform3f(location, v0, v1, v2);
        }

        void uniform3i(GLint location, GLint v0, GLint v1, GLint v2) override
        {
            log("glUniform3i({}, {}, {}, {})", location, v0, v1, v2);
            wrapper->uniform3i(location, v0, v1, v2);
        }

        void uniform3fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform3fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform3fv(location, count, value);
        }

        void uniform3iv(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform3iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform3iv(location, count, value);
        }

        void uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
        {
            log("glUniform4f({}, {}, {}, {}, {})", location, v0, v1, v2, v3);
            wrapper->uniform4f(location, v0, v1, v2, v3);
        }

        void uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) override
        {
            log("glUniform4i({}, {}, {}, {}, {})", location, v0, v1, v2, v3);
            wrapper->uniform4i(location, v0, v1, v2, v3);
        }

        void uniform4fv(GLint location, GLsizei count, const GLfloat* value) override
        {
            log("glUniform4fv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform4fv(location, count, value);
        }

        void uniform4iv(GLint location, GLsizei count, const GLint* value) override
        {
            log("glUniform4iv({}, {}, {})", location, count, static_cast<const void*>(value));
            wrapper->uniform4iv(location, count, value);
        }

        void uniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            log("glUniformMatrix2fv({}, {}, {}, {})", location, count, transpose, static_cast<const void*>(value));
            wrapper->uniformMatrix2fv(location, count, transpose, value);
        }

        void uniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            log("glUniformMatrix3fv({}, {}, {}, {})", location, count, transpose, static_cast<const void*>(value));
            wrapper->uniformMatrix3fv(location, count, transpose, value);
        }

        void uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
        {
            log("glUniformMatrix4fv({}, {}, {}, {})", location, count, transpose, static_cast<const void*>(value));
            wrapper->uniformMatrix4fv(location, count, transpose, value);
        }

        void useProgram(GLuint program) override
        {
            log("glUseProgram({})", program);
            wrapper->useProgram(program);
        }

        void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                 const void* pointer) override
        {
            log("glVertexAttribPointer({}, {}, {}, {}, {}, {})", index, size, type, normalized, stride, static_cast<const void*>(pointer));
            wrapper->vertexAttribPointer(index, size, type, normalized, stride, pointer);
        }

        void viewport(GLint x, GLint y, GLsizei width, GLsizei height) override
        {
            log("glViewport({}, {}, {}, {})", x, y, width, height);
            wrapper->viewport(x, y, width, height);
        }

        IOglWrapper* wrapper = nullptr;
    };
}
