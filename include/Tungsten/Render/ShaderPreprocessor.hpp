//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Tungsten
{
    class ShaderPreprocessor
    {
    public:
        ShaderPreprocessor();

        ShaderPreprocessor& add_define(std::string name,
                                       std::string value = {});

        ShaderPreprocessor& add_include(const std::string& name,
                                        const std::string& content);

        ShaderPreprocessor& add_include_path(const std::filesystem::path& path);

        ShaderPreprocessor& set_version(const std::string& version);

        [[nodiscard]] std::string
        preprocess(std::string_view source,
                   const std::filesystem::path& current_dir = {}) const;

        [[nodiscard]] std::string
        preprocess(const std::filesystem::path& path) const;

    private:
        [[nodiscard]] std::string
        get_include_file_content(const std::string& name,
                                 const std::filesystem::path& current_dir) const;

        std::string version_;
        std::unordered_map<std::string, std::string> includes_;
        std::vector<std::pair<std::string, std::string>> defines_;
        std::vector<std::filesystem::path> include_paths_;
    };
}
