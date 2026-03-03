//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>

#include "VertexAttributeType.hpp"
#include "Gl/GlVertexArray.hpp"

namespace Tungsten
{
    struct VertexAttribute
    {
        uint32_t location = UINT32_MAX;
        uint32_t buffer_id = 0;
        uint8_t count = 0;
        uint8_t offset = 0;
        uint8_t stride = 0;
        uint8_t divisor = 0;
        VertexAttributeDataType data_type = VertexAttributeDataType::FLOAT;
        VertexAttributeType type = VertexAttributeType::CUSTOM;
    };

    class VertexArrayObject
    {
    public:
        VertexArrayObject() = default;

        VertexArrayObject(VertexArrayHandle vao,
                          std::vector<VertexAttribute> attributes)
            : vao_(std::move(vao)),
              attributes_(std::move(attributes))
        {
        }

        ~VertexArrayObject() = default;

        VertexArrayObject(const VertexArrayObject&) = delete;

        VertexArrayObject& operator=(const VertexArrayObject&) = delete;

        VertexArrayObject(VertexArrayObject&&) noexcept = default;

        VertexArrayObject& operator=(VertexArrayObject&&) noexcept = default;

        void bind() const;

        [[nodiscard]] explicit operator bool() const;

        [[nodiscard]] uint32_t handle() const;

        [[nodiscard]] const std::vector<VertexAttribute>& attributes() const;

        [[nodiscard]] size_t num_buffers() const;

    private:
        VertexArrayHandle vao_;
        std::vector<VertexAttribute> attributes_;
    };
} // Tungsten
