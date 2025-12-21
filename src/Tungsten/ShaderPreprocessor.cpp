//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPreprocessor.hpp"

#include <fstream>

#include "Tungsten/DeviceInfo.hpp"
#include "Tungsten/TungstenException.hpp"
#include "Detail/DelimiterFinders.hpp"
#include "Detail/StringDelimiterIterator.hpp"

namespace Tungsten
{
    ShaderPreprocessor::ShaderPreprocessor()
        : version_(get_shader_language_version_string())
    {
    }

    void ShaderPreprocessor::add_define(std::string name, std::string value)
    {
        defines_.emplace_back(std::move(name), std::move(value));
    }

    void ShaderPreprocessor::add_include(const std::string& name, const std::string& content)
    {
        includes_[name] = content;
    }

    void ShaderPreprocessor::add_include_path(const std::filesystem::path& path)
    {
        include_paths_.push_back(path);
    }

    void ShaderPreprocessor::set_version(const std::string& version)
    {
        version_ = version;
    }

    namespace
    {
        std::string_view get_include_file_name(std::string_view line)
        {
            const auto first_quote = line.find_first_of('\"');
            if (first_quote == std::string_view::npos)
                return {};
            const auto second_quote = line.find_first_of('\"', first_quote + 1);
            if (second_quote == std::string_view::npos)
                return {};
            return line.substr(first_quote + 1,
                               second_quote - first_quote - 1);
        }
    }

    std::string
    ShaderPreprocessor::preprocess(std::string_view source,
                                   const std::filesystem::path& current_dir) const
    {
        std::string result;
        ParserTools::StringDelimiterIterator<ParserTools::FindNewline> it(source, {});
        while (it.next())
        {
            auto line = it.string();
            if (line.starts_with("#version"))
            {
                result.append("#version ");
                result.append(version_);
                result.push_back('\n');
                for (const auto& [name, value] : defines_)
                {
                    result.append("#define ");
                    result.append(name);
                    if (!value.empty())
                    {
                        result.push_back(' ');
                        result.append(value);
                    }
                    result.push_back('\n');
                }
            }
            else if (line.starts_with("#include"))
            {
                auto file_name = get_include_file_name(line);
                if (file_name.empty())
                    TUNGSTEN_THROW("Invalid include directive: " + std::string(line));

                auto content = get_include_file_content(std::string(file_name), current_dir);
                if (!content.empty())
                {
                    result.append(content);
                    if (!content.ends_with('\n'))
                        result.push_back('\n');
                }
            }
            else
            {
                result.append(line);
                result.push_back('\n');
            }
        }

        return result;
    }

    namespace
    {
        std::optional<std::string> read_file_content(const std::filesystem::path& path)
        {
            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file)
                return {};

            std::string content;
            file.seekg(0, std::ios::end);
            content.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&content[0], static_cast<std::streamsize>(content.size()));
            return content;
        }
    }

    std::string ShaderPreprocessor::preprocess(const std::filesystem::path& path) const
    {
        const auto content = read_file_content(path);
        if (!content)
            TUNGSTEN_THROW("Unable to read shader file: " + path.string());
        return preprocess(*content, path.parent_path());
    }

    std::string
    ShaderPreprocessor::get_include_file_content(const std::string& name,
                                                 const std::filesystem::path& current_dir) const
    {
        const auto include_it = includes_.find(std::string(name));
        if (include_it != includes_.end())
        {
            return include_it->second;
        }

        if (!current_dir.empty())
        {
            auto path = current_dir / name;
            if (auto content = read_file_content(path))
                return *content;
        }

        for (const auto& include_path : include_paths_)
        {
            auto path = include_path / name;
            if (auto content = read_file_content(path))
                return *content;
        }
        TUNGSTEN_THROW("Unable to find include file: " + name);
    }
}
