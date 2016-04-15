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
#include <memory>
#include <vector>

#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>

#include <reaver/exception.h>

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        struct type
        {
            const bool is_target_type = false;
        };

        using type_identifier = type *;

        class variable;
        class target;

        template<typename Tag>
        type_identifier get_type_identifier()
        {
            static type type_identifier = { std::is_base_of<target, Tag>::value };
            return &type_identifier;
        }

        class variable;
        // TODO: reaver::function instead of std::function once that thing is ready
        using constructor = std::function<std::shared_ptr<variable> (std::vector<std::shared_ptr<variable>>)>;

        struct type_descriptor
        {
            const std::u32string name;
            const std::string source_module;
            const constructor type_constructor;
        };

        namespace _detail
        {
            void _save_identifier(const std::vector<std::u32string> & name, type_identifier id);
            void _save_descriptor(type_identifier id, std::u32string name, std::string source_module, constructor type_constructor);
        }

        class duplicate_type_name : public exception
        {
        public:
            duplicate_type_name(std::u32string n, std::string sm) : exception{ logger::error }, name{ std::move(n) }, source_module{ std::move(sm) }
            {
                *this << "attempted to register a duplicate type name `" << boost::locale::conv::utf_to_utf<char>(name) << "`, originally registered by module `" << source_module << "`.";
            }

            const std::u32string name;
            const std::string source_module;
        };

        template<typename Tag>
        type_identifier create_type(std::u32string full_name, std::vector<std::u32string> name, std::string source_module, constructor type_constructor)
        {
            auto id = get_type_identifier<Tag>();

            if (type_constructor)
            {
                _detail::_save_identifier(name, id);
            }
            _detail::_save_descriptor(id, std::move(full_name), std::move(source_module), type_constructor);

            return id;
        }

        template<typename Tag>
        type_identifier create_type(std::u32string name, std::string source_module, constructor type_constructor)
        {
            std::vector<std::u32string> split;
            // TODO: need better split
            boost::algorithm::split(split, name, boost::is_any_of(U"."));

            return create_type<Tag>(std::move(name), std::move(split), std::move(source_module), std::move(type_constructor));
        }

        struct semantic_context;

        std::shared_ptr<variable> instantiate(const semantic_context & ctx, const std::vector<std::u32string> & name, std::vector<std::shared_ptr<variable>> variables);
        std::shared_ptr<variable> instantiate(type_identifier type, std::vector<std::shared_ptr<variable>> variables);

        std::shared_ptr<variable> global_context();
    }}
}

