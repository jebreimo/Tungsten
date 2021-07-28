//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-08-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    class RegularPolygonGenerator
    {
    public:
        double radius() const
        {
            return m_Radius;
        }

        RegularPolygonGenerator& setRadius(double radius)
        {
            m_Radius = radius;
            return *this;
        }

        double angle() const
        {
            return m_Angle;
        }

        RegularPolygonGenerator& setAngle(double angle)
        {
            m_Angle = angle;
            return *this;
        }

        const Xyz::Vector2d& center() const
        {
            return m_Center;
        }

        RegularPolygonGenerator& setCenter(const Xyz::Vector2d& center)
        {
            m_Center = center;
            return *this;
        }

        size_t numberOfPoints() const
        {
            return m_NumberOfPoints;
        }

        RegularPolygonGenerator& setNumberOfPoints(size_t numberOfPoints)
        {
            m_NumberOfPoints = numberOfPoints;
            return *this;
        }

        bool isClosed() const
        {
            return m_IsClosed;
        }

        RegularPolygonGenerator& setIsClosed(bool isClosed)
        {
            m_IsClosed = isClosed;
            return *this;
        }

        template <typename T>
        std::vector<Xyz::Vector<T, 2>> generate() const
        {
            if (m_NumberOfPoints < 3)
                return {};
            std::vector<Xyz::Vector<T, 2>> result;
            result.reserve(m_NumberOfPoints + (m_IsClosed ? 1 : 0));
            for (size_t i = 0; i < m_NumberOfPoints; ++i)
            {
                auto angle = m_Angle + 2 * i * Xyz::Constants<T>::PI / m_NumberOfPoints;
                result.push_back({T(m_Center[0] + m_Radius * cos(angle)),
                                  T(m_Center[1] + m_Radius * sin(angle))});
            }
            if (m_IsClosed && !result.empty())
                result.push_back(result.front());
            return result;
        }
    private:
        double m_Radius = 1.0;
        double m_Angle = 0.0;
        Xyz::Vector2d m_Center;
        size_t m_NumberOfPoints = 4;
        bool m_IsClosed = false;
    };
}
