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

#include <vector>

#include <reaver/variant.h>
#include <reaver/optional.h>

#include "lexer.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        struct context
        {
            std::vector<token>::iterator begin;
            std::vector<token>::iterator end;
        };

        inline token expect(context & ctx, token_type expected)
        {
            if (ctx.begin->type != expected)
            {
                std::terminate(); // need an actual handling here
            }

            if (ctx.begin == ctx.end)
            {
                std::terminate(); // and need it here too
            }

            return std::move(*ctx.begin++);
        }

        inline reaver::optional<token &> peek(context & ctx)
        {
            if (ctx.begin != ctx.end)
            {
                return { *ctx.begin };
            }

            return reaver::none;
        }

        inline reaver::optional<token &> peek(context & ctx, token_type expected)
        {
            if (ctx.begin != ctx.end && ctx.begin->type == expected)
            {
                return { *ctx.begin };
            }

            return reaver::none;
        }

        struct string
        {
            range_type range;
            token value;

            bool operator==(const string & other) const
            {
                return value.string == other.value.string;
            }
        };

        string parse_string(context & ctx);

        struct identifier
        {
            range_type range;
            token value;

            bool operator==(const identifier & other) const
            {
                return value.string == other.value.string;
            }
        };

        identifier parse_identifier(context & ctx);

        struct id_expression
        {
            range_type range;
            std::vector<identifier> identifiers;

            bool operator==(const id_expression & other) const
            {
                return identifiers == other.identifiers;
            }
        };

        id_expression parse_id_expression(context & ctx);

        struct instantiation
        {
            range_type range;
            id_expression type_name;
            std::vector<reaver::variant<
                string,
                id_expression,
                instantiation
            >> arguments;

            bool operator==(const instantiation & other) const
            {
                return type_name == other.type_name && arguments == other.arguments;
            }
        };

        reaver::variant<string, id_expression, instantiation> parse_expression(context & ctx);

        struct assignment
        {
            range_type range;
            id_expression lhs;
            reaver::variant<
                string, // just a value
                id_expression, // an alias
                instantiation // a new thingy
            > rhs;

            bool operator==(const assignment & other) const
            {
                return lhs == other.lhs && rhs == other.rhs;
            }
        };

        assignment parse_assignment(context & ctx);

        std::vector<assignment> parse(std::vector<token> tokens);
    }}
}

