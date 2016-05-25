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
#include <string>

#include <reaver/plugin.h>

#include "../parser/parser.h"
#include "../runtime/context.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class variable;
        class target;
        class delayed_variable;

        struct type;
        using type_identifier = type *;
        struct type_descriptor;

        struct semantic_context
        {
            semantic_context() = default;
            semantic_context(const semantic_context &) = default;
            semantic_context(semantic_context &&) = default;
            semantic_context & operator=(const semantic_context &) = default;
            semantic_context & operator=(semantic_context &&) = default;

            std::shared_ptr<variable> variables;
            std::unordered_map<std::u32string, std::shared_ptr<target>> targets;
            std::unordered_map<std::shared_ptr<delayed_variable>, range_type> unresolved;
            // ugly map because ugly incomplete type makes GCC unhappy when this is unordered
            std::map<type_identifier, type_descriptor> type_descriptors;
            std::unordered_set<plugin_function<void (context_ptr)>> plugin_initializers;
        };
    }}
}

