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

#include "despayre/runtime/context.h"
#include "despayre/semantics/variable.h"

namespace reaver
{
    namespace despayre
    {
        namespace cxx { inline namespace _v1
        {
            class cxx_compiler : public compiler
            {
            public:
                cxx_compiler(linker_capability cap, std::shared_ptr<variable> arguments) : _linker_cap{ std::move(cap) }, _arguments{ std::move(arguments) }
                {
                }

                virtual std::vector<boost::filesystem::path> inputs(context_ptr, const boost::filesystem::path &) const override;
                virtual std::vector<boost::filesystem::path> outputs(context_ptr, const boost::filesystem::path &) const override;

                virtual void build(context_ptr, const boost::filesystem::path &) const override;
                virtual const std::vector<linker_capability> & linker_caps(context_ptr, const::boost::filesystem::path &) const override
                {
                    return _linker_cap;
                }

            private:
                std::vector<linker_capability> _linker_cap;
                std::shared_ptr<variable> _arguments;
            };
        }}
    }
}

