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

#include <boost/locale.hpp>

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

        inline std::string utf8(std::experimental::u32string_view other)
        {
            return boost::locale::conv::utf_to_utf<char>(std::begin(other), std::end(other));
        }

        class expectation_failure : public exception
        {
        public:
            expectation_failure(token_type expected, std::experimental::u32string_view actual, range_type & r) : exception{ logger::fatal }
            {
                *this << r << ": expected `" << token_types[+expected] << "`, got `" << utf8(actual) << "`";
            }

            expectation_failure(std::experimental::string_view str, std::experimental::u32string_view actual, range_type & r) : exception{ logger::fatal }
            {
                *this <<  r << ": expected " << str << ", got `" << utf8(actual) << "`";
            }

            expectation_failure(token_type expected) : exception{ logger::fatal }
            {
                *this << "expected `" << token_types[+expected] << "`, got end of file";
            }

            expectation_failure(std::experimental::string_view str) : exception{ logger::fatal }
            {
                *this << "expected `" << str << "`, got end of file";
            }
        };

        inline token expect(context & ctx, token_type expected)
        {
            if (ctx.begin == ctx.end)
            {
                throw expectation_failure{ expected };
            }

            if (ctx.begin->type != expected)
            {
                throw expectation_failure{ expected, ctx.begin->string, ctx.begin->range };
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

        struct string_node
        {
            range_type range;
            token value;

            bool operator==(const string_node & other) const
            {
                return value.string == other.value.string;
            }
        };

        string_node parse_string(context & ctx);

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

        struct complex_expression;

        struct instantiation
        {
            range_type range;
            id_expression type_name;
            std::vector<reaver::variant<
                string_node,
                id_expression,
                instantiation,
                recursive_wrapper<complex_expression>
            >> arguments;

            bool operator==(const instantiation & other) const
            {
                return type_name == other.type_name && arguments == other.arguments;
            }
        };

        using expression = variant<
            string_node,
            id_expression,
            instantiation,
            recursive_wrapper<complex_expression>
        >;

        enum class operation_type
        {
            addition,
            removal
        };

        struct operation
        {
            range_type range;
            operation_type operation;
            expression operand;

            bool operator==(const struct operation & other) const
            {
                return operation == other.operation && operand == other.operand;
            }
        };

        std::vector<operation> parse_operations(context & ctx);

        struct complex_expression
        {
            range_type range;
            expression base;
            std::vector<operation> operations;

            bool operator==(const complex_expression & other) const
            {
                return base == other.base && operations == other.operations;
            }
        };

        expression parse_argument(context & ctx, bool complex = true);
        expression parse_expression(context & ctx, bool complex = true);

        enum class assignment_type
        {
            assignment,
            addition,
            removal
        };

        struct assignment
        {
            range_type range;
            id_expression lhs;
            assignment_type type;
            expression rhs;

            bool operator==(const assignment & other) const
            {
                return lhs == other.lhs && rhs == other.rhs;
            }
        };

        assignment parse_assignment(context & ctx);

        std::vector<assignment> parse(std::vector<token> tokens);
    }}
}

