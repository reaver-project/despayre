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

#include "despayre/semantics/semantics.h"
#include "despayre/semantics/target.h"
#include "despayre/semantics/string.h"
#include "despayre/semantics/delayed_variable.h"
#include "despayre/semantics/namespace.h"

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
            ctx.unresolved.emplace(unresolved);
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
                ctx.unresolved.emplace(std::dynamic_pointer_cast<delayed_variable>(instance));
            }

            return instance;
        },

        [&](const complex_expression & expr) -> std::shared_ptr<variable> {
            auto lhs = analyze_expression(ctx, expr.base);

            for (auto && op : expr.operations)
            {
                auto rhs = analyze_expression(ctx, op.operand);
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
                    ctx.unresolved.emplace(std::dynamic_pointer_cast<delayed_variable>(lhs));
                }
            }

            return lhs;
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
        class print : public target_clone_wrapper<print>
        {
        public:
            print(std::vector<std::shared_ptr<variable>> args) : target_clone_wrapper<print>{ get_type_identifier<print>() }, _args{ std::move(args) }
            {
            }

            print(const print &) = default;
            print(print &&) = default;

            virtual std::vector<std::shared_ptr<target>> dependencies() const override
            {
                return {};
            }

            virtual bool built() const override
            {
                return false;
            }

        protected:
            virtual void _build() override
            {
                for (auto && arg : _args)
                {
                    logger::dlog() << utf8(arg->as<string>()->value());
                }
            }

        private:
            std::vector<std::shared_ptr<variable>> _args;
        };

        auto register_print = once([]{
            create_type<print>(
                U"debug.print",
                "<builtin:debug>",
                make_type_checking_constructor<print>({
                    { get_type_identifier<string>(), 1 }
                })
            );
        });

        class aggregate : public target_clone_wrapper<aggregate>
        {
        public:
            aggregate(std::vector<std::shared_ptr<variable>> args) : target_clone_wrapper<aggregate>{ get_type_identifier<aggregate>() }
            {
                for (auto && arg : args)
                {
                    if (arg->type() && arg->type()->is_target_type)
                    {
                        _args.push_back(arg->as_target());
                    }
                }
            }

            virtual bool built() const override
            {
                return false;
            }

            virtual std::vector<std::shared_ptr<target>> dependencies() const override
            {
                return _args;
            }

        protected:
            virtual void _build() override
            {
            }

        private:
            std::vector<std::shared_ptr<target>> _args;
        };

        auto register_aggregate = once([]{
            create_type<aggregate>(
                U"aggregate",
                "<builtin>",
                make_type_checking_constructor<aggregate>({})
            );
        });
    }}
}

reaver::despayre::_v1::analysis_results reaver::despayre::_v1::analyze(const std::vector<reaver::despayre::_v1::assignment> & parse_tree)
{
    semantic_context ctx;
    ctx.variables = global_context();

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

    return { std::move(ctx.variables), std::move(ctx.targets) };
}

