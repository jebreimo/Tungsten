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
            : m_id()
        {}

        explicit GlHandle(GLuint id)
            : m_id(id)
        {}

        GlHandle(GlHandle&& other)
            : m_id(other.m_id)
        {
            other.m_id = 0;
        }

        ~GlHandle()
        {
            reset();
        }

        GlHandle& operator=(GlHandle&& other)
        {
            reset(other.m_id);
            other.m_id = 0;
            return *this;
        }

        operator GLuint() const
        {
            return m_id;
        }

        void reset(GLuint id = 0)
        {
            if (m_id)
                Deleter()(m_id);
            m_id = id;
        }

        GLuint release()
        {
            auto id = m_id;
            m_id = 0;
            return id;
        }
    private:
        GLuint m_id;
    };
}
