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

#include <reaver/unit.h>
#include <reaver/optional.h>

#include "despayre/parser/lexer.h"

using reaver::despayre::token_types;
using reaver::despayre::token_type;

std::array<std::string, +token_type::count> reaver::despayre::_v1::token_types;

static auto token_types_init = []() -> reaver::unit
{
    token_types[+token_type::identifier] = "identifier";
    token_types[+token_type::dot] = ".";
    token_types[+token_type::comma] = ",";
    token_types[+token_type::plus] = "+";
    token_types[+token_type::minus] = "-";
    token_types[+token_type::equals] = "=";
    token_types[+token_type::open_paren] = "(";
    token_types[+token_type::close_paren] = ")";
    token_types[+token_type::string] = "string";

    return {};
}();

const std::unordered_map<char32_t, token_type> reaver::despayre::_v1::symbols1 = {
    { '.', token_type::dot },
    { ',', token_type::comma },
    { '+', token_type::plus },
    { '-', token_type::minus },
    { '=', token_type::equals },
    { '(', token_type::open_paren },
    { ')', token_type::close_paren }
};

const std::unordered_map<char32_t, std::unordered_map<char32_t, token_type>> reaver::despayre::_v1::symbols2 = {
};

std::vector<reaver::despayre::_v1::token> reaver::despayre::_v1::tokenize(const std::experimental::u32string_view buildfile, const boost::filesystem::path & filename)
{
    position pos;
    pos.offset = -1;
    pos.column = 0;
    pos.line = 1;
    pos.file = filename.string();

    auto begin = std::begin(buildfile);
    auto end = std::end(buildfile);

    std::vector<token> tokens;

    auto get = [&]() -> reaver::optional<char32_t>
    {
        if (begin == end)
        {
            return reaver::none;
        }

        if (*begin == U'\n')
        {
            pos.column = 0;
            ++pos.line;
        }

        ++pos.offset;
        ++pos.column;
        return *begin++;
    };

    auto peek = [&](std::size_t x = 0) -> reaver::optional<char32_t>
    {
        if (static_cast<std::size_t>(std::distance(begin, end)) < x)
        {
            return reaver::none;
        }

        auto ret = begin;
        std::advance(ret, x); // this interface is absurdly bad
        return *ret;
    };

    auto generate_token = [&](token_type type, position begin, position end, std::u32string string)
    {
        tokens.emplace_back(token{ type, std::move(string), range_type{ begin, end } });
    };

    auto is_white_space = [](char32_t c)
    {
        return c == U' ' || c == U'\t' || c == '\n' || c == U'\r';
    };

    auto is_identifier_start = [](char32_t c)
    {
        return (c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z') || c == U'_';
    };

    auto is_decimal = [](char32_t c)
    {
        return c >= U'0' && c <= U'9';
    };

    auto is_identifier_char = [=](char32_t c)
    {
        return is_identifier_start(c) || is_decimal(c);
    };

    while (begin != end)
    {
        auto next = get();

        if (is_white_space(*next))
        {
            continue;
        }

        auto p = pos;
        if (next == U'/')
        {
            auto second = peek();

            if (second == U'/')
            {
                while ((next = get()) && *next != U'\n')
                {
                }

                continue;
            }

            if (second == U'*')
            {
                get();

                while ((next = get()) && (second = peek()) && next != U'*' && second != U'/')
                {
                }

                if (next && second && next == U'*' && second == U'/')
                {
                    get();
                    continue;
                }

                throw unterminated_comment{ { p, pos } };
            }
        }

        {
            auto second = peek();

            if (second && symbols2.find(*next) != symbols2.end() && symbols2.at(*next).find(*second) != symbols2.at(*next).end())
            {
                auto p = pos;
                generate_token(symbols2.at(*next).at(*second), p, p + 2, { *next, *get() });
                continue;
            }

            else if (symbols1.find(*next) != symbols1.end())
            {
                auto p = pos;
                generate_token(symbols1.at(*next), p, p + 1, { *next });
                continue;
            }
        }

        std::u32string variable_length;

        if (next == U'"')
        {
            auto second = peek();

            while (next && second && (second != U'"' || next == U'\\') && (second != U'\n' || next == U'\\'))
            {
                variable_length.push_back(*next);

                next = get();
                second = peek();
            }

            if (!next || second == U'\n')
            {
                throw unterminated_string{ { p, p + variable_length.size() } };
            }

            variable_length.push_back(*next);
            get();

            generate_token(token_type::string, p, p + variable_length.size(), std::move(variable_length).substr(1));
            continue;
        }

        if (is_identifier_start(*next))
        {
            do
            {
                variable_length.push_back(*next);
            } while (peek() && is_identifier_char(*peek()) && (next = get()));

            generate_token(token_type::identifier, p, p + variable_length.size(), std::move(variable_length));
            continue;
        }

        throw exception{ logger::fatal } << range_type{ pos, pos } << ": unexpected character: `" << *next << "`";
    }

    return tokens;
}

