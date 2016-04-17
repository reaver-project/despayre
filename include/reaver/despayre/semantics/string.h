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

#include "variable.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class string : public clone_wrapper<string>
        {
        public:
            string(std::u32string value) : clone_wrapper<string>{ get_type_identifier<string>() }, _value{ std::move(value) }
            {
                _add_addition(get_type_identifier<string>(), [](_op_arg lhs, _op_arg rhs) -> std::shared_ptr<variable> {
                    auto lhs_string = lhs->as<string>();
                    auto rhs_string = rhs->as<string>();

                    return std::make_shared<string>(lhs_string->value() + rhs_string->value());
                });
            }

            string(const string &) = default;
            string(string &&) = default;

            const std::u32string & value() const
            {
                return _value;
            }

        private:
            std::u32string _value;
        };

        namespace _detail
        {
            static auto _register_string = once([]{ create_type<string>(U"string", "<builtin>", nullptr); });
        }
    }}
}

