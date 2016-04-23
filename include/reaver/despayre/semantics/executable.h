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

#include "target.h"
#include "set.h"
#include "string.h"
#include "files.h"
#include "delayed_variable.h" // FIXME: make_type_checking_constructor forces this include

namespace reaver
{
    namespace despayre { inline namespace _v
    {
        class executable : public target_clone_wrapper<executable>
        {
        public:
            executable(std::vector<std::shared_ptr<variable>> arguments) : target_clone_wrapper<executable>{ get_type_identifier<executable>() }
            {
                for (auto && arg : arguments)
                {
                    type_dispatch(arg,
                        id<string>(), [&](std::shared_ptr<string> arg) {
                            _name = arg->value();
                            return unit{};
                        },

                        id<files>(), [&](std::shared_ptr<files> arg) {
                            std::copy(arg->value().begin(), arg->value().end(), std::back_inserter(_deps));
                            return unit{};
                        }
                    );
                }
            }

            virtual std::vector<std::shared_ptr<target>> dependencies() const override
            {
                return _deps;
            }

            virtual bool built() const override
            {
                return false;
            }

        protected:
            virtual void _build() override
            {
                logger::dlog() << "Building executable " << utf8(_name) << ".";
            }

        private:
            std::u32string _name;
            std::vector<std::shared_ptr<target>> _deps;
        };

        namespace _detail
        {
            static auto _register_executable = reaver::once([]{
                create_type<executable>(
                    U"executable",
                    "<builtin>",
                    make_type_checking_constructor<executable>({
                        { get_type_identifier<set>(), {} },
                        { get_type_identifier<string>(), 1 }
                    })
                );
            });
        }
    }}
}

