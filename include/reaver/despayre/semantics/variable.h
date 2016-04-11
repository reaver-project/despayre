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

#include "type.h"
#include "../parser/parser.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <reaver/once.h>
#include <reaver/optional.h>
#include <reaver/future.h>

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

        // ...in Vapor this will be a typeclass
        // ...but doing that kind of thing in C++ manually is troublesome
        // and I really don't want to dive into boost.type_erasure or some other dark magic library right now
        class variable : public std::enable_shared_from_this<variable>
        {
        public:
            friend class delayed_variable;

            variable(type_identifier type_id) : _type_id{ type_id }
            {
            }

            virtual ~variable()
            {
            }

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
            std::shared_ptr<T> as() const
            {
                if (type() != get_type_identifier<T>())
                {
                    throw std::bad_cast{};
                }

                return std::dynamic_pointer_cast<const T>(_shared_this());
            }

            virtual std::shared_ptr<target> as_target()
            {
                if (type() && type()->is_target_type)
                {
                    return std::dynamic_pointer_cast<target>(_shared_this());
                }

                assert(!"nope");
            }

            std::shared_ptr<variable> operator+=(const std::shared_ptr<variable> & other)
            {
                _operator_plus_call_table.at(other->_type_id)(*this, other);
                return shared_from_this();
            }

            std::shared_ptr<variable> operator+(const std::shared_ptr<variable> & other) const
            {
                auto clone = _clone();
                return *clone += other;
            }

            std::shared_ptr<variable> operator-=(const std::shared_ptr<variable> & other)
            {
                _operator_minus_call_table.at(other->_type_id)(*this, other);
                return shared_from_this();
            }

            std::shared_ptr<variable> operator-(const std::shared_ptr<variable> & other) const
            {
                auto clone = _clone();
                return *clone -= other;
            }

            virtual void add_property(std::u32string name, std::shared_ptr<variable> value)
            {
                throw invalid_add_property{ std::move(name) };
            }

            virtual std::shared_ptr<variable> get_property(const std::u32string & name) const
            {
                throw invalid_get_property{ name };
            }

        protected:
            virtual std::shared_ptr<variable> _clone() const = 0;

            virtual std::shared_ptr<variable> _shared_this()
            {
                return shared_from_this();
            }

            virtual std::shared_ptr<const variable> _shared_this() const
            {
                return shared_from_this();
            }

        private:
            using _operator_type = void (const variable &, const std::shared_ptr<variable> &);

            type_identifier _type_id;

            std::unordered_map<type_identifier, _operator_type *> _operator_plus_call_table;
            std::unordered_map<type_identifier, _operator_type *> _operator_minus_call_table;
        };

        class target : public variable
        {
        public:
            target(type_identifier type_id) : variable{ type_id }
            {
            }

            virtual bool built() const = 0;

            future<> build()
            {
                if (!_build_future)
                {
                    _build_future = when_all(fmap(dependencies(), [](auto && dep){ return dep->build(); }))
                        .then([&](){ _build(); });
                }

                return *_build_future;
            }

            virtual std::vector<std::shared_ptr<target>> dependencies() const = 0;

        protected:
            virtual void _build() = 0;

            optional<future<>> _build_future;
        };

        namespace _detail
        {
            template<typename T, typename Base>
            class _clone_wrapper : public Base
            {
            protected:
                _clone_wrapper(type_identifier id) : Base{ id }
                {
                }

                virtual std::shared_ptr<variable> _clone() const override
                {
                    auto & t = dynamic_cast<const T &>(*this);
                    return std::make_shared<T>(t);
                }
            };
        }

        template<typename T>
        using clone_wrapper = _detail::_clone_wrapper<T, variable>;

        template<typename T>
        using target_clone_wrapper = _detail::_clone_wrapper<T, target>;

        class string : public clone_wrapper<string>
        {
        public:
            string(std::u32string value) : clone_wrapper<string>{ get_type_identifier<string>() }, _value{ std::move(value) }
            {
            }

            string(const string &) = default;
            string(string &&) = default;

            const std::u32string & value() const
            {
                return _value;
            }

        private:
            std::u32string _value;
        };

        class set : public clone_wrapper<set>
        {
        public:
            set(std::unordered_set<std::shared_ptr<variable>> variables) : clone_wrapper<set>{ get_type_identifier<set>() }, _value{ std::move(variables) }
            {
            }

        private:
            std::unordered_set<std::shared_ptr<variable>> _value;
        };

        namespace _detail
        {
            static auto _register_string = once([]{ create_type<string>(U"string", "<builtin>", nullptr); });
            static auto _register_set = once([]{ create_type<string>(U"set", "<builtin>", nullptr); });
        }

        class delayed_variable;

        struct semantic_context
        {
            std::unordered_map<std::u32string, std::shared_ptr<variable>> variables;
            std::unordered_map<std::u32string, std::shared_ptr<target>> targets;
            std::unordered_set<std::shared_ptr<delayed_variable>> unresolved;
        };

        class delayed_variable : public variable
        {
        public:
            delayed_variable(type_identifier actual_type, std::vector<std::shared_ptr<variable>> args) : variable{ nullptr }, _state{ _delayed_instantiation_info{ actual_type, std::move(args) } }
            {
            }

            delayed_variable(std::vector<std::u32string> ref_id_expr) : variable{ nullptr }, _state{ _delayed_reference_info{ std::move(ref_id_expr) } }
            {
            }

            virtual type_identifier type() const override
            {
                if (_state.index() == 2)
                {
                    return get<2>(_state)->type();
                }

                return nullptr;
            }

            bool try_resolve(semantic_context & ctx)
            {
                return get<0>(fmap(_state, make_overload_set(
                    [&](const std::shared_ptr<variable> &) {
                        return false;
                    },

                    [&](_delayed_reference_info & info) {
                        auto var_it = ctx.variables.find(info.referenced_id_expression.front());
                        if (var_it != ctx.variables.end())
                        {
                            auto val = var_it->second;
                            for (auto i = 1ull; i < info.referenced_id_expression.size(); ++i)
                            {
                                val = val->get_property(info.referenced_id_expression[i]);
                            }

                            _state = val;
                            ctx.unresolved.erase(ctx.unresolved.find(std::dynamic_pointer_cast<delayed_variable>(shared_from_this())));
                            return true;
                        }

                        return false;
                    },

                    [&](_delayed_instantiation_info & info) {
                        if (std::count_if(info.arguments.begin(), info.arguments.end(), [](auto && arg) { return arg->type() == nullptr; }) == 0)
                        {
                            _state = instantiate(info.actual_type, info.arguments);
                            assert(get<2>(_state)->type());
                            ctx.unresolved.erase(ctx.unresolved.find(std::dynamic_pointer_cast<delayed_variable>(shared_from_this())));
                            return true;
                        }

                        return false;
                    }
                )));
            }

            virtual std::shared_ptr<target> as_target() override
            {
                return get<0>(fmap(_state, make_overload_set(
                    [&](const std::shared_ptr<variable> & resolved) {
                        return resolved->as_target();
                    },

                    [&](const _delayed_reference_info &) -> std::shared_ptr<target> {
                        assert(!"");
                    },

                    [&](const _delayed_instantiation_info &) -> std::shared_ptr<target> {
                        assert(!"");
                    }
                )));
            }

        protected:
            virtual std::shared_ptr<variable> _clone() const override
            {
                return get<0>(fmap(_state, make_overload_set(
                    [&](const std::shared_ptr<variable> & resolved) {
                        return resolved->_clone();
                    },

                    [&](const _delayed_reference_info & ref_info) -> std::shared_ptr<variable> {
                        return std::make_shared<delayed_variable>(ref_info.referenced_id_expression);
                    },

                    [&](const _delayed_instantiation_info & inst_info) -> std::shared_ptr<variable> {
                        return std::make_shared<delayed_variable>(inst_info.actual_type, inst_info.arguments);
                    }
                )));
            }

            virtual std::shared_ptr<variable> _shared_this() override
            {
                if (_state.index() == 2)
                {
                    return get<2>(_state);
                }

                return shared_from_this();
            }

            virtual std::shared_ptr<const variable> _shared_this() const override
            {
                if (_state.index() == 2)
                {
                    return get<2>(_state);
                }

                return shared_from_this();
            }

        private:
            struct _delayed_instantiation_info
            {
                type_identifier actual_type;
                std::vector<std::shared_ptr<variable>> arguments;
            };

            struct _delayed_reference_info
            {
                std::vector<std::u32string> referenced_id_expression;
            };

            variant<
                _delayed_instantiation_info,
                _delayed_reference_info,
                std::shared_ptr<variable>
            > _state;
        };

        class name_space // silly keywords
            : public clone_wrapper<name_space>
        {
        public:
            name_space() : clone_wrapper<name_space>{ get_type_identifier<name_space>() }
            {
            }

            virtual void add_property(std::u32string name, std::shared_ptr<variable> value) override
            {
                auto & variable = _map[std::move(name)];
                if (variable)
                {
                    assert(!"do something in this case");
                }
                variable = std::move(value);
            }

            virtual std::shared_ptr<variable> get_property(const std::u32string & name) const override
            {
                auto it = _map.find(name);
                if (it == _map.end())
                {
                    return nullptr;
                }
                return it->second;
            }

        private:
            std::unordered_map<std::u32string, std::shared_ptr<variable>> _map;
        };

        namespace _detail
        {
            static auto _register_namespace = once([]{ create_type<name_space>(U"namespace", "<builtin>", nullptr); });
        }

        template<typename T>
        std::shared_ptr<variable> default_constructor(std::vector<std::shared_ptr<variable>> variables)
        {
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

                return std::make_shared<T>(std::move(variables));
            };
        }
    }}
}

