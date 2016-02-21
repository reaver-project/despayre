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

#include "despayre/semantics/type.h"
#include "despayre/semantics/variable.h"

#include <unordered_map>

using type_identifier = reaver::despayre::_v1::type_identifier;
using type_descriptor = reaver::despayre::_v1::type_descriptor;
using variable_ptr = std::shared_ptr<reaver::despayre::_v1::variable>;

namespace
{
    std::unordered_map<type_identifier, type_descriptor> _type_descriptors;
    std::unordered_map<std::u32string, type_identifier> _type_identifiers;
}

void reaver::despayre::_v1::_detail::_save_identifier(std::u32string name, type_identifier id)
{
    if (_type_identifiers.find(name) != _type_identifiers.end())
    {
        const auto & source_module = _type_descriptors.at(_type_identifiers.at(name)).source_module;
        throw duplicate_type_name{ std::move(name), source_module };
    }

    _type_identifiers.emplace(std::move(name), id);
}

void reaver::despayre::_v1::_detail::_save_descriptor(type_identifier id, std::u32string name, std::string source_module, constructor type_constructor)
{
    _type_descriptors.emplace(id, type_descriptor{ std::move(name), std::move(source_module), type_constructor });
}

variable_ptr reaver::despayre::_v1::instantiate(const std::u32string & name, std::vector<variable_ptr> variables)
{
    return _type_descriptors.at(_type_identifiers.at(name)).type_constructor(std::move(variables));
}

variable_ptr reaver::despayre::_v1::instantiate(type_identifier id, std::vector<variable_ptr> variables)
{
    return _type_descriptors.at(id).type_constructor(std::move(variables));
}

