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

#include <memory>
#include <unordered_map>
#include <mutex>

#include <boost/filesystem.hpp>

#include <reaver/optional.h>
#include <reaver/future.h>

#include "compiler.h" // declares context_ptr

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class target;

        struct runtime_context
        {
            runtime_context(boost::filesystem::path output_dir) : output_directory{ std::move(output_dir) }
            {
            }

            const boost::filesystem::path output_directory;

            std::mutex futures_lock;
            std::unordered_map<std::shared_ptr<target>, optional<future<>>> build_futures;

            std::unordered_map<boost::filesystem::path, std::shared_ptr<target>, boost::hash<boost::filesystem::path>> generated_files;
            std::unordered_map<boost::filesystem::path, std::shared_ptr<target>, boost::hash<boost::filesystem::path>> file_targets;

            compiler_configuration compilers;
        };

        inline context_ptr make_runtime_context(boost::filesystem::path output_dir)
        {
            return std::make_shared<runtime_context>(std::move(output_dir));
        }
    }}
}

