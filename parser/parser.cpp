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

#include "parser.h"

std::vector<reaver::despayre::assignment> reaver::despayre::_v1::parse(std::vector<reaver::despayre::token> tokens)
{
    std::vector<assignment> assignments;

    context ctx;
    ctx.begin = std::begin(tokens);
    ctx.end = std::end(tokens);

    while (ctx.begin != ctx.end)
    {
        assignments.push_back(parse_assignment(ctx));
    }

    return assignments;
}

reaver::despayre::assignment reaver::despayre::_v1::parse_assignment(reaver::despayre::context & ctx)
{
    static std::unordered_map<token_type, assignment_type> assignments = {
        { token_type::equals, assignment_type::assignment },
        { token_type::plus_equals, assignment_type::addition },
        { token_type::minus_equals, assignment_type::removal }
    };

    auto id = parse_id_expression(ctx);

    auto peeked = peek(ctx);
    if (!peeked)
    {
        throw expectation_failure{ "assignment-operator" };
    }

    auto it = assignments.find(peeked->type);
    if (it == assignments.end())
    {
        throw expectation_failure{ "assignment-operator", peeked->string, ctx.begin->range };
    }
    auto op = expect(ctx, peeked->type);

    auto value = parse_expression(ctx);

    return { range_type{ id.range.start(), get<0>(fmap(value, [](auto && v){ return v.range.end(); })) }, std::move(id), it->second, std::move(value) };
}

reaver::despayre::id_expression reaver::despayre::_v1::parse_id_expression(reaver::despayre::context & ctx)
{
    std::vector<identifier> identifiers;
    identifiers.push_back(parse_identifier(ctx));

    while (peek(ctx, token_type::dot))
    {
        expect(ctx, token_type::dot);
        identifiers.push_back(parse_identifier(ctx));
    }

    return { range_type{ identifiers.front().range.start(), identifiers.back().range.end() }, std::move(identifiers) };
}

reaver::despayre::identifier reaver::despayre::_v1::parse_identifier(reaver::despayre::context & ctx)
{
    auto token = expect(ctx, token_type::identifier);
    return { std::move(token.range), std::move(token) };
}

reaver::despayre::string reaver::despayre::_v1::parse_string(reaver::despayre::context & ctx)
{
    auto token = expect(ctx, token_type::string);
    return { std::move(token.range), std::move(token) };
}

using expression = reaver::variant<
    reaver::despayre::string,
    reaver::despayre::id_expression,
    reaver::despayre::instantiation
>;

expression reaver::despayre::_v1::parse_expression(reaver::despayre::context & ctx)
{
    auto peeked = peek(ctx);
    if (!peeked)
    {
        throw expectation_failure{ "expression" };
    }

    switch (peeked->type)
    {
        case token_type::string:
        {
            return { parse_string(ctx) };
        }

        case token_type::identifier:
        {
            auto id = parse_id_expression(ctx);
            std::vector<expression> arguments;

            if (peek(ctx, token_type::open_paren))
            {
                expect(ctx, token_type::open_paren);

                if (peek(ctx) && peek(ctx)->type != token_type::close_paren)
                {
                    arguments.push_back(parse_expression(ctx));

                    while (peek(ctx) && peek(ctx)->type != token_type::close_paren)
                    {
                        expect(ctx, token_type::comma);
                        arguments.push_back(parse_expression(ctx));
                    }
                }

                auto close = expect(ctx, token_type::close_paren);
                return { instantiation{ range_type{ id.range.start(), close.range.end() }, std::move(id), std::move(arguments) } };
            }

            else
            {
                return { std::move(id) };
            }
        }

        default:
            throw expectation_failure{ "expression", peeked->string, ctx.begin->range };
    }
}

