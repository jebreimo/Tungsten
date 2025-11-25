//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-25.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

namespace
{
    class OpenGlWrapper
    {
    public:
        virtual void activeTexture() = 0;
        virtual void attachShader() = 0;
        virtual void bindBuffer() = 0;
        virtual void bindFramebuffer() = 0;
        virtual void bindTexture() = 0;
        virtual void bindVertexArray() = 0;
        virtual void blendFunc() = 0;
        virtual void bufferData() = 0;
        virtual void bufferSubData() = 0;
        virtual void checkFramebufferStatus() = 0;
        virtual void clear() = 0;
        virtual void clearColor() = 0;
        virtual void compileShader() = 0;
        virtual void copyTexSubImage2D() = 0;
        virtual void createProgram() = 0;
        virtual void createShader() = 0;
        virtual void deleteBuffers() = 0;
        virtual void deleteFramebuffers() = 0;
        virtual void deleteProgram() = 0;
        virtual void deleteShader() = 0;
        virtual void deleteTextures() = 0;
        virtual void deleteVertexArrays() = 0;
        virtual void disable() = 0;
        virtual void disableVertexAttribArray() = 0;
        virtual void drawArrays() = 0;
        virtual void drawElements() = 0;
        virtual void enable() = 0;
        virtual void enableVertexAttribArray() = 0;
        virtual void finish() = 0;
        virtual void framebufferTexture2D() = 0;
        virtual void genBuffers() = 0;
        virtual void generateMipmap() = 0;
        virtual void genFramebuffers() = 0;
        virtual void genTextures() = 0;
        virtual void genVertexArrays() = 0;
        virtual void getAttribLocation() = 0;
        virtual void getBufferParameteriv() = 0;
        virtual void getError() = 0;
        virtual void getIntegerv() = 0;
        virtual void getProgramInfoLog() = 0;
        virtual void getProgramiv() = 0;
        virtual void getShaderInfoLog() = 0;
        virtual void getShaderiv() = 0;
        virtual void getTexParameterfv() = 0;
        virtual void getTexParameteriv() = 0;
        virtual void getUniformLocation() = 0;
        virtual void isBuffer() = 0;
        virtual void isEnabled() = 0;
        virtual void linkProgram() = 0;
        virtual void shaderSource() = 0;
        virtual void texImage2D() = 0;
        virtual void texParameterf() = 0;
        virtual void texParameteri() = 0;
        virtual void texStorage2D() = 0;
        virtual void texSubImage2D() = 0;
        virtual void uniform1f() = 0;
        virtual void uniform1i() = 0;
        virtual void uniform2fv() = 0;
        virtual void uniform2iv() = 0;
        virtual void uniform3fv() = 0;
        virtual void uniform3iv() = 0;
        virtual void uniform4fv() = 0;
        virtual void uniform4iv() = 0;
        virtual void uniformMatrix2fv() = 0;
        virtual void uniformMatrix3fv() = 0;
        virtual void uniformMatrix4fv() = 0;
        virtual void useProgram() = 0;
        virtual void vertexAttribPointer() = 0;
        virtual void viewport() = 0;
    };
}