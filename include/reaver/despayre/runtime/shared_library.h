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

#include "../semantics/target.h"
#include "../semantics/string.h"
#include "files.h"

namespace reaver
{
    namespace despayre { inline namespace _v
    {
        class shared_library : public target
        {
        public:
            shared_library(std::vector<std::shared_ptr<variable>> arguments) : target{ get_type_identifier<shared_library>() }
            {
                for (auto && arg : arguments)
                {
                    type_dispatch(arg,
                        id<string>(), [&](std::shared_ptr<string> arg) {
                            _name = arg->value();
                            return unit{};
                        },

                        id<files>(), [&](std::shared_ptr<files> arg) {
                            _deps.push_back(std::move(arg));
                            return unit{};
                        }
                    );
                }
            }

            virtual const std::vector<std::shared_ptr<target>> & dependencies(context_ptr) override
            {
                return _deps;
            }

            virtual bool built(context_ptr) override
            {
                return false;
            }

        protected:
            virtual void _build(context_ptr ctx) override
            {
                logger::dlog() << "Building shared library " << filesystem::make_relative(ctx->output_directory / ("lib" + utf8(_name) + ".so")).string() << ".";
            }

        private:
            std::u32string _name;
            std::vector<std::shared_ptr<target>> _deps;
        };
    }}
}

