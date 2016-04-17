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

#include "despayre/semantics/delayed_variable.h"

bool reaver::despayre::_v1::delayed_variable::try_resolve(reaver::despayre::_v1::semantic_context & ctx)
{
    return get<0>(fmap(_state, make_overload_set(
        [&](const std::shared_ptr<variable> &) {
            return false;
        },

        [&](_delayed_reference_info & info) {
            auto val = ctx.variables;
            for (auto i = 0ull; i < info.referenced_id_expression.size() && val; ++i)
            {
                val = val->get_property(info.referenced_id_expression[i]);
            }

            if (!val)
            {
                return false;
            }

            _state = val;
            ctx.unresolved.erase(ctx.unresolved.find(std::dynamic_pointer_cast<delayed_variable>(shared_from_this())));
            return true;
        },

        [&](_delayed_instantiation_info & info) {
            if (std::count_if(info.arguments.begin(), info.arguments.end(), [](auto && arg) { return arg->type() == nullptr; }) == 0)
            {
                _state = instantiate(info.actual_type, info.arguments);
                assert(get<0>(_state)->type());
                ctx.unresolved.erase(ctx.unresolved.find(std::dynamic_pointer_cast<delayed_variable>(shared_from_this())));
                return true;
            }

            return false;
        },

        [&](_delayed_type_info & info) {
            // wait until all arguments are resolved
            if (std::count_if(info.arguments.begin(), info.arguments.end(), [](auto && variable) { return variable->type() == nullptr; }))
            {
                return false;
            }

            auto val = ctx.variables;
            for (auto i = 0ull; i < info.type_name.size() && val; ++i)
            {
                val = val->get_property(info.type_name[i]);
            }

            if (!val)
            {
                return false;
            }

            if (val->type() != get_type_identifier<type_descriptor_variable>())
            {
                assert(!"fdsa");
            }

            _state = instantiate(val->as<type_descriptor_variable>()->identifier(), std::move(info.arguments));
            ctx.unresolved.erase(ctx.unresolved.find(std::dynamic_pointer_cast<delayed_variable>(shared_from_this())));
            return true;
        },

        [&](_delayed_operation_info & info) {
            if (info.lhs->type() == nullptr || info.rhs->type() == nullptr)
            {
                return false;
            }

            switch (info.operation)
            {
                case operation_type::addition:
                    _state = *info.lhs + info.rhs;
                    break;

                case operation_type::removal:
                    _state = *info.lhs - info.rhs;
                    break;
            }

            ctx.unresolved.erase(ctx.unresolved.find(std::dynamic_pointer_cast<delayed_variable>(shared_from_this())));
            return true;
        }
    )));
}

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::delayed_variable::clone() const
{
    return get<0>(fmap(_state, make_overload_set(
        [&](const std::shared_ptr<variable> & resolved) {
            return resolved->clone();
        },

        [&](const _delayed_reference_info & ref_info) -> std::shared_ptr<variable> {
            return std::make_shared<delayed_variable>(ref_info.referenced_id_expression);
        },

        [&](const _delayed_instantiation_info & inst_info) -> std::shared_ptr<variable> {
            return std::make_shared<delayed_variable>(inst_info.actual_type, inst_info.arguments);
        },

        [&](const _delayed_type_info & type_info) -> std::shared_ptr<variable> {
            return std::make_shared<delayed_variable>(type_info.type_name, type_info.arguments);
        },

        [&](const _delayed_operation_info & op_info) -> std::shared_ptr<variable> {
            return std::make_shared<delayed_variable>(op_info.lhs, op_info.rhs, op_info.operation);
        }
    )));
}

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::delayed_addition(std::shared_ptr<reaver::despayre::_v1::variable> lhs, std::shared_ptr<reaver::despayre::_v1::variable> rhs)
{
    return std::make_shared<delayed_variable>(std::move(lhs), std::move(rhs), operation_type::addition);
}

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::delayed_removal(std::shared_ptr<reaver::despayre::_v1::variable> lhs, std::shared_ptr<reaver::despayre::_v1::variable> rhs)
{
    return std::make_shared<delayed_variable>(std::move(lhs), std::move(rhs), operation_type::removal);
}

