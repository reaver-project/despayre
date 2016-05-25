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

#include <reaver/prelude/functor.h>
#include <reaver/overloads.h>

#include "despayre/semantics/semantics.h"
#include "despayre/semantics/target.h"
#include "despayre/semantics/string.h"
#include "despayre/semantics/delayed_variable.h"
#include "despayre/semantics/namespace.h"

using var_map = std::unordered_map<std::u32string, std::shared_ptr<reaver::despayre::_v1::variable>>;

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::analyze_expression(reaver::despayre::_v1::semantic_context & ctx, const reaver::despayre::_v1::expression & expr)
{
    auto lhs = analyze_simple_expression(ctx, expr.base);

    for (auto && op : expr.operations)
    {
        auto rhs = analyze_simple_expression(ctx, op.operand);
        switch (op.operation)
        {
            case operation_type::addition:
                lhs = *lhs + rhs;
                break;

            case operation_type::removal:
                lhs = *lhs - rhs;
                break;
        }

        if (lhs->type() == nullptr)
        {
            ctx.unresolved.emplace(std::dynamic_pointer_cast<delayed_variable>(lhs), range_type{ expr.range.start(), op.range.end() });
        }
    }

    return lhs;
}

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::analyze_simple_expression(semantic_context & ctx, const reaver::despayre::_v1::simple_expression & expr)
{
    return get<0>(fmap(expr, make_overload_set(
        [&](const string_node & str) -> std::shared_ptr<variable> {
            return std::make_shared<string>(str.value.string);
        },

        [&](const id_expression & expr) -> std::shared_ptr<variable> {
            auto val = ctx.variables;
            for (auto i = 0ull; i < expr.identifiers.size() && val; ++i)
            {
                val = val->get_property(expr.identifiers[i].value.string);
            }

            if (val)
            {
                return val;
            }

            auto unresolved = std::make_shared<delayed_variable>(
                fmap(expr.identifiers, [](auto && ident) { return ident.value.string; })
            );
            ctx.unresolved.emplace(unresolved, expr.range);
            return unresolved;
        },

        [&](const instantiation & inst) -> std::shared_ptr<variable> {
            auto instance = instantiate(
                ctx,
                fmap(inst.type_name.identifiers, [](auto && arg){ return arg.value.string; }),
                fmap(inst.arguments, [&](auto && arg) { return analyze_expression(ctx, arg); })
            );

            if (instance->type() == nullptr)
            {
                // ugly; figure out a better way to do this
                // without spilling semantic_context to the constructor
                ctx.unresolved.emplace(std::dynamic_pointer_cast<delayed_variable>(instance), inst.range);
            }

            return instance;
        },


        [&](const auto & arg) -> std::shared_ptr<variable> {
            std::cout << typeid(arg).name() << std::endl;
            assert(0);
        }
    )));
}

reaver::despayre::_v1::semantic_context reaver::despayre::_v1::analyze(const std::vector<reaver::despayre::_v1::assignment> & parse_tree)
{
    semantic_context ctx;
    ctx.variables = std::make_shared<name_space>();
    register_builtins(ctx);

    for (auto && assignment : parse_tree)
    {
        auto rhs_value = analyze_expression(ctx, assignment.rhs);
        auto & lhs = assignment.lhs;
        auto val = ctx.variables;

        for (auto i = 0ull; i < lhs.identifiers.size() - 1; ++i)
        {
            auto nested = val->get_property(lhs.identifiers[i].value.string);
            if (nested)
            {
                val = nested;
                continue;
            }

            auto ns = std::make_shared<name_space>();
            val->add_property(lhs.identifiers[i].value.string, ns);
            val = ns;
        }

        val->add_property(lhs.identifiers.back().value.string, rhs_value);

        if (rhs_value->type() && rhs_value->type()->is_target_type)
        {
            ctx.targets.emplace(
                boost::join(fmap(assignment.lhs.identifiers, [](auto && i) { return i.value.string; }), U"."),
                std::dynamic_pointer_cast<target>(rhs_value));
        }
    }

    std::size_t previous = 0;
    while (previous != ctx.unresolved.size())
    {
        previous = ctx.unresolved.size();

        for (auto && u : ctx.unresolved)
        {
            if (u.first->try_resolve(ctx))
            {
                break;
            }
        }
    }

    if (!ctx.unresolved.empty())
    {
        throw exception{ logger::fatal } << "some variables could not have been resolved; first at " << ctx.unresolved.begin()->second;
    }

    return ctx;
}

