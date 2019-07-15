//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 19.04.2017.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <GL/glew.h>

namespace Tungsten
{
    template <typename Deleter>
    class GlHandle
    {
    public:
        GlHandle()
            : m_Id()
        {}

        GlHandle(GLuint id)
            : m_Id(id)
        {}

        GlHandle(GlHandle&& other)
            : m_Id(other.m_Id)
        {
            other.m_Id = 0;
        }

        ~GlHandle()
        {
            reset();
        }

        GlHandle& operator=(GlHandle&& other)
        {
            reset(other.m_Id);
            other.m_Id = 0;
            return *this;
        }

        operator GLuint() const
        {
            return m_Id;
        }

        void reset(GLuint id = 0)
        {
            if (m_Id)
                Deleter()(m_Id);
            m_Id = id;
        }

        GLuint release()
        {
            auto id = m_Id;
            m_Id = 0;
            return id;
        }
    private:
        GLuint m_Id;
    };
}
