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

#include "../include/reaver/despayre/semantics/semantics.h"

#include <reaver/prelude/functor.h>
#include <reaver/overloads.h>

using var_map = std::unordered_map<std::u32string, std::shared_ptr<reaver::despayre::_v1::variable>>;

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::analyze_expression(semantic_context & ctx, const reaver::despayre::_v1::expression & expr)
{
    return get<0>(fmap(expr, make_overload_set(
        [&](const string_node & str) -> std::shared_ptr<variable> {
            return std::make_shared<string>(str.value.string);
        },

        [&](const id_expression & expr) -> std::shared_ptr<variable> {
            assert(expr.identifiers.size() == 1);

            auto var_it = ctx.variables.find(expr.identifiers.front().value.string);

            if (var_it != ctx.variables.end())
            {
                return var_it->second;
            }

            auto unresolved = std::make_shared<delayed_variable>(
                fmap(expr.identifiers, [](auto && ident) { return ident.value.string; })
            );
            ctx.unresolved.emplace(unresolved);
            return unresolved;
        },

        [&](const instantiation & inst) -> std::shared_ptr<variable> {
            assert(inst.type_name.identifiers.size() == 1);
            auto instance = instantiate(
                inst.type_name.identifiers.front().value.string,
                fmap(inst.arguments, [&](auto && arg) { return analyze_expression(ctx, arg); })
            );

            if (instance->type() == nullptr)
            {
                // ugly; figure out a better way to do this
                // without spilling semantic_context to the constructor
                ctx.unresolved.emplace(std::dynamic_pointer_cast<delayed_variable>(instance));
            }

            return instance;
        },

        [&](const auto & arg) -> std::shared_ptr<variable> {
            std::cout << typeid(arg).name() << std::endl;
            assert(0);
        }
    )));
}

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class print : public clone_wrapper<print>
        {
        public:
            print(std::vector<std::shared_ptr<variable>> args) : clone_wrapper<print>{ get_type_identifier<print>() }
            {
                for (auto && arg : args)
                {
                    logger::dlog() << utf8(arg->as<string>()->value());
                }
            }

            print(const print &) = default;
            print(print &&) = default;
        };

        auto register_print = once([]{
            create_type<print>(
                U"print",
                "<builtin>",
                make_type_checking_constructor<print>({
                    { get_type_identifier<string>(), 1 }
                })
            );
        });
    }}
}

var_map reaver::despayre::_v1::analyze(const std::vector<reaver::despayre::_v1::assignment> & parse_tree)
{
    semantic_context ctx;

    for (auto && assignment : parse_tree)
    {
        auto rhs_value = analyze_expression(ctx, assignment.rhs);

        switch (assignment.type)
        {
            case assignment_type::assignment:
                assert(assignment.lhs.identifiers.size() == 1);
                ctx.variables.emplace(assignment.lhs.identifiers.front().value.string, rhs_value);

            case assignment_type::addition:
                assert(1);

            case assignment_type::removal:
                assert(2);
        }
    }

    std::size_t previous = 0;
    while (previous != ctx.unresolved.size())
    {
        previous = ctx.unresolved.size();

        for (auto && u : ctx.unresolved)
        {
            if (u->try_resolve(ctx))
            {
                break;
            }
        }
    }

    if (!ctx.unresolved.empty())
    {
        throw exception{ logger::fatal } << "some variables could not have been resolved";
    }

    return std::move(ctx.variables);
}
