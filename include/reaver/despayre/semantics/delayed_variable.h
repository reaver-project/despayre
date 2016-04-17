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

#include "variable.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class delayed_variable : public variable
        {
        public:
            delayed_variable(type_identifier actual_type, std::vector<std::shared_ptr<variable>> args) : variable{ nullptr }, _state{ _delayed_instantiation_info{ actual_type, std::move(args) } }
            {
            }

            delayed_variable(std::vector<std::u32string> ref_id_expr) : variable{ nullptr }, _state{ _delayed_reference_info{ std::move(ref_id_expr) } }
            {
            }

            delayed_variable(std::vector<std::u32string> type_name, std::vector<std::shared_ptr<variable>> arguments) : variable{ nullptr }, _state{ _delayed_type_info{ std::move(type_name), std::move(arguments) } }
            {
            }

            delayed_variable(std::shared_ptr<variable> lhs, std::shared_ptr<variable> rhs, operation_type op) : variable{ nullptr }, _state{ _delayed_operation_info{ std::move(lhs), std::move(rhs), op } }
            {
            }

            virtual type_identifier type() const override
            {
                if (_state.index() == 0)
                {
                    return get<0>(_state)->type();
                }

                return nullptr;
            }

            bool try_resolve(semantic_context & ctx);

            virtual std::shared_ptr<target> as_target() override
            {
                return get<0>(fmap(_state, make_overload_set(
                    [&](const std::shared_ptr<variable> & resolved) {
                        return resolved->as_target();
                    },

                    [&](const auto &) -> std::shared_ptr<target> {
                        assert(!"");
                    }
                )));
            }

            virtual std::shared_ptr<variable> clone() const override;

        protected:
            virtual std::shared_ptr<variable> _shared_this() override
            {
                if (_state.index() == 0)
                {
                    return get<0>(_state);
                }

                return shared_from_this();
            }

            virtual std::shared_ptr<const variable> _shared_this() const override
            {
                if (_state.index() == 0)
                {
                    return get<0>(_state);
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

            struct _delayed_type_info
            {
                std::vector<std::u32string> type_name;
                std::vector<std::shared_ptr<variable>> arguments;
            };

            struct _delayed_operation_info
            {
                std::shared_ptr<variable> lhs;
                std::shared_ptr<variable> rhs;
                operation_type operation;
            };

            variant<
                std::shared_ptr<variable>,
                _delayed_instantiation_info,
                _delayed_reference_info,
                _delayed_type_info,
                _delayed_operation_info
            > _state;
        };
    }}
}

