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

#include <unordered_set>
#include <algorithm>

#include "variable.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class set : public clone_wrapper<set>
        {
        public:
            set(std::unordered_set<std::shared_ptr<variable>> variables) : clone_wrapper<set>{ get_type_identifier<set>() }, _value{ std::move(variables) }
            {
                _add_addition(get_type_identifier<set>(), [](_op_arg lhs, _op_arg rhs) -> std::shared_ptr<variable> {
                    auto & lhs_set = lhs->as<set>()->value();
                    auto & rhs_set = rhs->as<set>()->value();

                    // ...need a better set_union
                    std::unordered_set<std::shared_ptr<variable>> result;
                    std::set_union(lhs_set.begin(), lhs_set.end(), rhs_set.begin(), rhs_set.end(), std::inserter(result, result.begin()));
                    return std::make_shared<set>(std::move(result));
                });

                _add_removal(get_type_identifier<set>(), [](_op_arg lhs, _op_arg rhs) -> std::shared_ptr<variable> {
                    auto & lhs_set = lhs->as<set>()->value();
                    auto & rhs_set = rhs->as<set>()->value();

                    // ...need a better set_difference
                    std::unordered_set<std::shared_ptr<variable>> result;
                    std::set_difference(lhs_set.begin(), lhs_set.end(), rhs_set.begin(), rhs_set.end(), std::inserter(result, result.begin()));
                    return std::make_shared<set>(std::move(result));
                });
            }

            const std::unordered_set<std::shared_ptr<variable>> & value() const
            {
                return _value;
            }

        private:
            std::unordered_set<std::shared_ptr<variable>> _value;
        };

        namespace _detail
        {
            static auto _register_set = once([]{ create_type<string>(U"set", "<builtin>", nullptr); });
        }
    }}
}

