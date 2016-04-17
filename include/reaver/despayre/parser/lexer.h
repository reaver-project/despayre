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

#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <experimental/string_view>

#include <boost/filesystem.hpp>

#include <reaver/exception.h>

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        struct position
        {
            std::size_t offset = 0;
            std::size_t line = 1;
            std::size_t column = 1;
            std::string file;

            std::size_t operator-(const position & rhs) const
            {
                return offset - rhs.offset;
            }

            position & operator+=(std::size_t count)
            {
                offset += count;
                column += count;
                return *this;
            }

            position operator+(std::size_t count)
            {
                auto ret = *this;
                ret += count;
                return ret;
            }
        };

        class range_type
        {
        public:
            range_type()
            {
            }

            range_type(position start, position end) : _start{ std::move(start) }, _end{ std::move(end) }
            {
            }

            const position & start() const
            {
                return _start;
            }

            const position & end() const
            {
                return _end;
            }

        private:
            position _start;
            position _end;
        };

        inline std::ostream & operator<<(std::ostream & os, const range_type & r)
        {
            if (r.end() - r.start() > 1)
            {
                return os << r.start().line << ":" << r.start().column << " (" << r.start().offset << ") - " << r.end().line << ":" << r.end().column << " (" << r.end().offset << ")";
            }

            return os << r.start().line << ":" << r.start().column << " (" << r.start().offset << ")";
        }

        enum class token_type
        {
            identifier,
            dot,
            comma,
            plus,
            minus,
            equals,
            open_paren,
            close_paren,
            string,

            count
        };

        constexpr std::size_t operator+(token_type type)
        {
            return static_cast<std::size_t>(type);
        }

        extern std::array<std::string, +token_type::count> token_types;
        extern const std::unordered_map<char32_t, token_type> symbols1;
        extern const std::unordered_map<char32_t, std::unordered_map<char32_t, token_type>> symbols2;

        struct token
        {
            token(token_type type, std::u32string string, range_type range) : type{ type }, string{ std::move(string) }, range{ std::move(range) }
            {
            }

            token_type type;
            std::u32string string;
            range_type range;
        };

        std::vector<token> tokenize(const std::experimental::u32string_view buildfile, const boost::filesystem::path & filename);

        class unterminated_comment : public reaver::exception
        {
        public:
            unterminated_comment(range_type r) : exception{ logger::error }, range{ std::move(r) }
            {
                *this << "unterminated comment at " << range;
            }

            range_type range;
        };

        class unterminated_string : public reaver::exception
        {
        public:
            unterminated_string(range_type r) : exception{ logger::error }, range{ std::move(r) }
            {
                *this << "unterminated string at " << range;
            }

            range_type range;
        };
    }}
}

