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

#include <reaver/future.h>

#include "variable.h"
#include "../runtime/context.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class target : public variable
        {
        public:
            target(type_identifier type_id) : variable{ type_id }
            {
            }

            virtual bool built(context_ptr) = 0;

            future<> build(context_ptr ctx)
            {
                auto this_target = _shared_this()->as_target();
                if (this_target->built(ctx))
                {
                    return make_ready_future();
                }

                auto & build_future = ctx->build_futures[this_target];
                if (!build_future)
                {
                    build_future = when_all(fmap(dependencies(ctx), [ctx](auto && dep){ return dep->build(ctx); }))
                        .then([&, ctx](){ _build(ctx); });
                }

                return *build_future;
            }

            virtual const std::vector<std::shared_ptr<target>> & dependencies(context_ptr)
            {
                static std::vector<std::shared_ptr<target>> empty;
                return empty;
            }

            virtual void invalidate()
            {
            }

        protected:
            virtual void _build(context_ptr) = 0;
        };
    }}
}

