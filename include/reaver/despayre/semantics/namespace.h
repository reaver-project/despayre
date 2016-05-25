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

#include "variable.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class name_space // silly keywords
            : public variable
        {
        public:
            name_space() : variable{ get_type_identifier<name_space>() }
            {
            }

            virtual void add_property(std::u32string name, std::shared_ptr<variable> value) override
            {
                auto & variable = _map[std::move(name)];
                if (variable)
                {
                    assert(!"do something in this case");
                }
                variable = std::move(value);
            }

            virtual std::shared_ptr<variable> get_property(const std::u32string & name) const override
            {
                auto it = _map.find(name);
                if (it == _map.end())
                {
                    return nullptr;
                }
                return it->second;
            }

        private:
            std::unordered_map<std::u32string, std::shared_ptr<variable>> _map;
        };
    }}
}

