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

#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include <boost/filesystem.hpp>

#include <reaver/prelude/monad.h>

#include "decl.h"
#include "flags.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        struct linker_description;
        using linker_capability = std::shared_ptr<linker_description>;

        class linker;

        struct linker_description
        {
            std::string name;
            std::shared_ptr<linker> convenient_linker;
            std::vector<std::string> compatible_with;
            std::vector<std::string> inconvenient_linker_flags;
        };

        class linker
        {
        public:
            virtual ~linker() = default;

            void build(context_ptr ctx, const boost::filesystem::path & output, binary_type type, const std::vector<boost::filesystem::path> & inputs, const std::vector<linker_capability> & required_caps) const
            {
                std::stringstream all_flags{ " " }; // really need sane ranges though
                std::vector<std::string> empty; // curses to C++ lambda return type deduction and the retarded {} rules
                for (auto && flag : mbind(required_caps, [&](auto && cap) { return &*cap->convenient_linker != this ? cap->inconvenient_linker_flags : empty; }))
                {
                    all_flags << std::quoted(std::move(flag)) << " ";
                }

                _build(ctx, output, type, inputs, all_flags.str());
            }

        protected:
            virtual void _build(context_ptr, const boost::filesystem::path &, binary_type, const std::vector<boost::filesystem::path> &, const std::string &) const = 0;
        };

        class linker_configuration
        {
        public:
            void register_linker(linker_capability linker)
            {
                _linkers.push_back(std::move(linker));
            }

            std::shared_ptr<linker> get_linker(const std::vector<linker_capability> & caps)
            {
                if (caps.size() == 0)
                {
                    assert(!"no caps set; add default linker");
                }

                if (caps.size() == 1)
                {
                    return caps.front()->convenient_linker;
                }

                auto it = std::find_if(_linkers.begin(), _linkers.end(), [&](auto && linker_cap) {
                    for (auto && cap : caps)
                    {
                        if (std::find_if(linker_cap->compatible_with.begin(), linker_cap->compatible_with.end(), [&](auto && elem){ return cap->name == elem; }) == linker_cap->compatible_with.end())
                        {
                            return false;
                        }
                    }

                    return true;
                });

                if (it == _linkers.end())
                {
                    assert(!"no linker found; add default linker");
                }

                return (*it)->convenient_linker;
            }

        private:
            std::vector<linker_capability> _linkers;
        };
    }}
}

