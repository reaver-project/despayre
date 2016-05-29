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

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <reaver/once.h>
#include <reaver/optional.h>

#include "type.h"
#include "context.h"
#include "../parser/parser.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class invalid_add_property : public exception
        {
        public:
            invalid_add_property(std::u32string name) : exception{ logger::error }, name{ std::move(name) }
            {
                *this << "failed to add property `" << utf8(name) << " `to a variable.";
            }

            std::u32string name;
        };

        class invalid_get_property : public exception
        {
        public:
            invalid_get_property(std::u32string name) : exception{ logger::error }, name{ std::move(name) }
            {
                *this << "failed to get property `" << utf8(name) << " `from a variable.";
            }

            std::u32string name;
        };

        class delayed_variable;
        class target;

        std::shared_ptr<variable> delayed_addition(std::shared_ptr<variable> lhs, std::shared_ptr<variable> rhs);
        std::shared_ptr<variable> delayed_removal(std::shared_ptr<variable> lhs, std::shared_ptr<variable> rhs);

        // ...in Vapor this will be a typeclass
        // ...but doing that kind of thing in C++ manually is troublesome
        // and I really don't want to dive into boost.type_erasure or some other dark magic library right now
        class variable : public std::enable_shared_from_this<variable>
        {
        public:
            friend class delayed_variable;

            variable(type_identifier type_id) : _type_id{ type_id }
            {
                _add_addition(nullptr, delayed_addition);
                _add_removal(nullptr, delayed_removal);
            }

            virtual ~variable() = default;

            virtual type_identifier type() const
            {
                return _type_id;
            }

            template<typename T>
            std::shared_ptr<T> as()
            {
                if (type() != get_type_identifier<T>())
                {
                    throw std::bad_cast{};
                }

                return std::dynamic_pointer_cast<T>(_shared_this());
            }

            template<typename T>
            std::shared_ptr<const T> as() const
            {
                if (type() != get_type_identifier<T>())
                {
                    throw std::bad_cast{};
                }

                return std::dynamic_pointer_cast<const T>(_shared_this());
            }

            virtual std::shared_ptr<target> as_target();

            std::shared_ptr<variable> operator+(std::shared_ptr<variable> other);
            std::shared_ptr<variable> operator-(std::shared_ptr<variable> other);

            virtual void add_property(std::u32string name, std::shared_ptr<variable> value)
            {
                throw invalid_add_property{ std::move(name) };
            }

            virtual std::shared_ptr<variable> get_property(const std::u32string & name) const
            {
                throw invalid_get_property{ name };
            }

        protected:
            using _op_arg = std::shared_ptr<variable>;
            using _operator_type = std::shared_ptr<variable> (_op_arg, _op_arg);

            virtual std::shared_ptr<variable> _shared_this()
            {
                return shared_from_this();
            }

            virtual std::shared_ptr<const variable> _shared_this() const
            {
                return shared_from_this();
            }

            void _add_addition(type_identifier other_id, _operator_type handler)
            {
                _operator_plus_call_table.emplace(other_id, handler);
            }

            void _add_removal(type_identifier other_id, _operator_type handler)
            {
                _operator_minus_call_table.emplace(other_id, handler);
            }

        private:
            type_identifier _type_id;

            std::unordered_map<type_identifier, _operator_type *> _operator_plus_call_table;
            std::unordered_map<type_identifier, _operator_type *> _operator_minus_call_table;
        };

        class type_descriptor_variable : public variable
        {
        public:
            type_descriptor_variable(type_identifier id) : variable{ reaver::despayre::get_type_identifier<type_descriptor_variable>() }, _identifier{ id }
            {
            }

            type_identifier identifier() const
            {
                return _identifier;
            }

        private:
            type_identifier _identifier;
        };

        template<typename T>
        std::shared_ptr<variable> default_constructor(std::vector<std::shared_ptr<variable>> variables)
        {
            if (std::count_if(variables.begin(), variables.end(), [](auto && variable) { return variable->type() == nullptr; }))
            {
                return std::make_shared<delayed_variable>(get_type_identifier<T>(), std::move(variables));
            }

            return std::make_shared<T>(std::move(variables));
        }

        template<typename T>
        auto make_type_checking_constructor(std::unordered_map<type_identifier, optional<std::size_t>> type_specifiers)
        {
            return [type_specifiers = std::move(type_specifiers)](std::vector<std::shared_ptr<variable>> variables) -> std::shared_ptr<variable> {
                if (std::count_if(variables.begin(), variables.end(), [](auto && variable) { return variable->type() == nullptr; }))
                {
                    return std::make_shared<delayed_variable>(get_type_identifier<T>(), std::move(variables));
                }

                for (auto && type_specifier : type_specifiers)
                {
                    auto actual = std::count_if(variables.begin(), variables.end(), [&](auto && variable) { return variable->type() == type_specifier.first; });
                    if (type_specifier.second && type_specifier.second != actual)
                    {
                        assert(0);
                    }
                }

                for (auto && argument : variables)
                {
                    auto it = type_specifiers.find(argument->type());
                    if (it == type_specifiers.end() || it->second == 0)
                    {
                        assert(0);
                    }
                }

                return std::make_shared<T>(std::move(variables));
            };
        }

        template<typename Type, typename F>
        decltype(auto) type_dispatch(std::shared_ptr<variable> var, id<Type>, F && callback)
        {
            if (var->type() == get_type_identifier<Type>())
            {
                return std::forward<F>(callback)(var->as<Type>());
            }

            assert(!"invalid dispatch");
        }

        template<typename Type, typename F, typename... Tail, typename std::enable_if<(sizeof...(Tail) > 0), int>::type = 0>
        decltype(auto) type_dispatch(std::shared_ptr<variable> var, id<Type>, F && head_callback, Tail &&... tail)
        {
            if (var->type() == get_type_identifier<Type>())
            {
                return std::forward<F>(head_callback)(var->as<Type>());
            }

            return type_dispatch(std::move(var), std::forward<Tail>(tail)...);
        }
    }}
}

