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

#include "target.h"
#include "string.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class print : public target
        {
        public:
            print(std::vector<std::shared_ptr<variable>> args) : target{ get_type_identifier<print>() }, _args{ std::move(args) }
            {
            }

            print(const print &) = default;
            print(print &&) = default;

            virtual bool built(context_ptr) override
            {
                return false;
            }

        protected:
            virtual void _build(context_ptr) override
            {
                for (auto && arg : _args)
                {
                    logger::dlog() << utf8(arg->as<string>()->value());
                }
            }

        private:
            std::vector<std::shared_ptr<variable>> _args;
        };

        class aggregate : public target
        {
        public:
            aggregate(std::vector<std::shared_ptr<variable>> args) : target{ get_type_identifier<aggregate>() }
            {
                for (auto && arg : args)
                {
                    if (arg->type() && arg->type()->is_target_type)
                    {
                        _args.push_back(arg->as_target());
                    }
                }
            }

            virtual bool built(context_ptr) override
            {
                return false;
            }

            virtual const std::vector<std::shared_ptr<target>> & dependencies(context_ptr) override
            {
                return _args;
            }

        protected:
            virtual void _build(context_ptr) override
            {
            }

        private:
            std::vector<std::shared_ptr<target>> _args;
        };

    }}
}

