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

#include <reaver/plugin.h>

#include "variable.h"
#include "namespace.h"
#include "context.h"
#include "string.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        struct import_tag {};

        class plugin_namespace : public variable
        {
        public:
            plugin_namespace(std::shared_ptr<plugin> plugin) : variable{ get_type_identifier<plugin_namespace>() }, _plugin{ std::move(plugin) }
            {
            }

        private:
            std::shared_ptr<plugin> _plugin;
        };

        auto generate_import(semantic_context & ctx)
        {
            return [&ctx](std::vector<std::shared_ptr<variable>> args)
            {
                assert(args.size() == 2);
                assert(args[0]->type() == get_type_identifier<string>());

                auto plugin = open_library("despayre." + utf8(args[0]->as<string>()->value()));
                plugin->get_symbol<void (semantic_context &)>("init_semantic")(ctx);
                ctx.plugin_initializers.insert({ { plugin, "init_runtime" }, args[1] });

                return std::make_shared<plugin_namespace>(std::move(plugin));
            };
        }
    }}
}

