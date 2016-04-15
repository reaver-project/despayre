/**
 * Despayre License
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/

#pragma once

#include <string>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <reaver/optional.h>

#include "parser/parser.h"
#include "semantics/semantics.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class despayre
        {
        public:
            despayre(boost::filesystem::path buildfile_path, boost::filesystem::path cwd = boost::filesystem::current_path()) : despayre{ _load_file(buildfile_path), std::move(buildfile_path), std::move(cwd) }
            {
            }

            despayre(std::u32string buildfile_contents, boost::filesystem::path buildfile_path, boost::filesystem::path cwd = boost::filesystem::current_path()) : _buildfile_path{ std::move(buildfile_path) }, _working_directory{ std::move(cwd) }, _buildfile{ std::move(buildfile_contents) }
            {
                _parse_tree = parse(tokenize(_buildfile, _buildfile_path));
                auto analyzed = analyze(_parse_tree);
                _variables = std::move(analyzed.variables);
                _targets = std::move(analyzed.targets);
            }

            // TODO: this also should return a future
            // not block like a dumb temporary implementation that it is
            void build(std::string target_name)
            {
                std::u32string converted = boost::locale::conv::utf_to_utf<char32_t>(target_name);

                std::shared_ptr<target> target;
                if (_targets.find(converted) != _targets.end())
                {
                    target = _targets.at(converted);
                }
                else
                {
                    std::vector<std::u32string> identifiers;
                    // need a better split
                    boost::algorithm::split(identifiers, converted, boost::is_any_of(U"."));

                    auto variable = _variables;
                    for (auto i = 0ull; i < identifiers.size() && variable; ++i)
                    {
                        variable = variable->get_property(identifiers[i]);
                    }

                    if (variable && variable->type()->is_target_type)
                    {
                        target = variable->as_target();
                    }
                }

                if (!target)
                {
                    // TODO: exception type
                    throw exception{ logger::fatal } << "could not find the requested target `" << target_name << "`.";
                }

                if (!target->built())
                {
                    auto future = target->build();
                    while (!future.try_get())
                    {
                        std::this_thread::yield();
                    }
                }
            }

        private:
            boost::filesystem::path _buildfile_path;
            boost::filesystem::path _working_directory;
            boost::filesystem::path _output_directory = boost::filesystem::current_path() / "build-output";

            std::u32string _buildfile;
            std::vector<assignment> _parse_tree;

            std::shared_ptr<variable> _variables;
            std::unordered_map<std::u32string, std::shared_ptr<target>> _targets;

            static std::u32string _load_file(const boost::filesystem::path & buildfile_path)
            {
                std::ifstream input{ buildfile_path.string() };
                std::string buffer_utf8{ std::istreambuf_iterator<char>{ input.rdbuf() }, std::istreambuf_iterator<char>{} };
                std::u32string input_content = boost::locale::conv::utf_to_utf<char32_t>(buffer_utf8);

                return input_content;
            }
        };
    }}
}

