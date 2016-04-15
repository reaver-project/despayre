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
    class _type_descriptor_variable : public reaver::despayre::clone_wrapper<_type_descriptor_variable>
    {
    public:
        _type_descriptor_variable(type_identifier id) : reaver::despayre::clone_wrapper<_type_descriptor_variable>{ reaver::despayre::get_type_identifier<_type_descriptor_variable>() }, _identifier{ id }
        {
        }

        type_identifier identifier() const
        {
            return _identifier;
        }

    private:
        type_identifier _identifier;
    };

    std::unordered_map<type_identifier, type_descriptor> _type_descriptors;
    std::shared_ptr<reaver::despayre::variable>_global_context = std::make_shared<reaver::despayre::name_space>();
    std::atomic<bool> _global_context_accessed{ false };
}

void reaver::despayre::_v1::_detail::_save_identifier(const std::vector<std::u32string> & name, type_identifier id)
{
    if (_global_context_accessed)
    {
        assert(!"throw a runtime error; this is an invalid use of the library");
    }

    auto val = _global_context;
    for (auto i = 0ull; i < name.size() - 1; ++i)
    {
        auto nested = val->get_property(name[i]);
        if (nested)
        {
            val = nested;
            continue;
        }

        auto ns = std::make_shared<name_space>();
        val->add_property(name[i], ns);
        val = ns;
    }

    val->add_property(name.back(), std::make_shared<_type_descriptor_variable>(id));
}

void reaver::despayre::_v1::_detail::_save_descriptor(type_identifier id, std::u32string name, std::string source_module, constructor type_constructor)
{
    _type_descriptors.emplace(id, type_descriptor{ std::move(name), std::move(source_module), type_constructor });
}

variable_ptr reaver::despayre::_v1::instantiate(const semantic_context & ctx, const std::vector<std::u32string> & name, std::vector<variable_ptr> variables)
{
    auto val = ctx.variables;
    for (auto i = 0ull; i < name.size() && val; ++i)
    {
        val = val->get_property(name[i]);
    }

    if (!val)
    {
        assert(!"asdf");
    }

    if (val->type() != get_type_identifier<_type_descriptor_variable>())
    {
        assert(!"fdsa");
    }

    return instantiate(val->as<_type_descriptor_variable>()->identifier(), std::move(variables));
}

variable_ptr reaver::despayre::_v1::instantiate(type_identifier id, std::vector<variable_ptr> variables)
{
    return _type_descriptors[id].type_constructor(std::move(variables));
}

variable_ptr reaver::despayre::_v1::global_context()
{
    auto copy = _global_context->clone();
    _global_context_accessed = true;
    return copy;
}

