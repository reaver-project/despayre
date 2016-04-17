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

        template<typename T>
        using target_clone_wrapper = _detail::_clone_wrapper<T, target>;
    }}
}

