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

#include <boost/filesystem.hpp>

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        // this file is included from runtime/context.h
        // don't be silly and don't introduce cyclic dependencies
        struct runtime_context;
        using context_ptr = std::shared_ptr<runtime_context>;

        class compiler
        {
        public:
            virtual ~compiler() = default;

            virtual std::vector<boost::filesystem::path> inputs(context_ptr, const boost::filesystem::path &) const = 0;
            virtual std::vector<boost::filesystem::path> outputs(context_ptr, const boost::filesystem::path &) const = 0;

            virtual void build(context_ptr, const boost::filesystem::path &) const = 0;
        };

        using compiler_ptr = std::shared_ptr<compiler>;

        class compiler_configuration
        {
        public:
            void register_compiler(boost::filesystem::path ext, compiler_ptr compiler)
            {
                auto & entry = _configuration[ext.extension()];
                if (entry)
                {
                    assert(!"should throw an exception here, I guess");
                }
                entry = compiler;
            }

            compiler_ptr get_compiler(boost::filesystem::path ext) const
            {
                return _configuration.at(ext.extension());
            }

        private:
            std::unordered_map<boost::filesystem::path, compiler_ptr, boost::hash<boost::filesystem::path>> _configuration;
        };
    }}
}

