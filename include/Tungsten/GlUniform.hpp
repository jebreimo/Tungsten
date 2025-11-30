//****************************************************************************
// Copyright © 2015 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2015-01-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Xyz.hpp>

namespace Tungsten
{
    void set_uniform(int32_t location, int32_t val1);

    void set_uniform(int32_t location, int32_t val1, int32_t val2);

    void set_uniform(int32_t location, int32_t val1, int32_t val2, int32_t val3);

    void set_uniform(int32_t location, int32_t val1, int32_t val2,
                     int32_t val3, int32_t val4);

    void set_uniform(int32_t location, float val1);

    void set_uniform(int32_t location, float val1, float val2);

    void set_uniform(int32_t location, float val1, float val2, float val3);

    void set_uniform(int32_t location, float val1, float val2,
                     float val3, float val4);

    void set_uniform(int32_t location, const Xyz::Vector2I& vec);

    void set_uniform(int32_t location, const Xyz::Vector3I& vec);

    void set_uniform(int32_t location, const Xyz::Vector4I& vec);

    void set_uniform(int32_t location, const Xyz::Vector2F& vec);

    void set_uniform(int32_t location, const Xyz::Vector3F& vec);

    void set_uniform(int32_t location, const Xyz::Vector4F& vec);

    void set_uniform(int32_t location, Xyz::Matrix2F mat, bool transpose = true);

    void set_uniform(int32_t location, Xyz::Matrix3F mat, bool transpose = true);

    void set_uniform(int32_t location, Xyz::Matrix4F mat, bool transpose = true);

    template <typename T>
    class Uniform
    {
    public:
        Uniform()
                : location_(-1)
        {}

        explicit Uniform(int32_t location)
                : location_(location)
        {}

        void set(const T& value)
        {
            set_uniform(location_, value);
        }
    private:
        int32_t location_;
    };
}
