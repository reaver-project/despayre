/**
 * Despayre License
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

#include "despayre/semantics/context.h"
#include "despayre/runtime/context.h"

#include "compiler.h"
#include "linker.h"

namespace reaver
{
    namespace despayre
    {
        namespace cxx { inline namespace _v1
        {
            extern "C" void init_semantic(reaver::despayre::_v1::semantic_context &)
            {
            }

            extern "C" void init_runtime(reaver::despayre::_v1::context_ptr ctx, std::shared_ptr<variable> arguments)
            {
                auto linker = std::make_shared<linker_description>();
                linker->name = "c++";
                linker->convenient_linker = std::make_shared<cxx_linker>(arguments);
                linker->compatible_with = {};
                linker->inconvenient_linker_flags = { "-lstdc++" };

                ctx->linkers.register_linker(linker);

                auto comp = std::make_shared<cxx_compiler>(std::move(linker), arguments);
                ctx->compilers.register_compiler(".cpp", comp);
                ctx->compilers.register_compiler(".cxx", comp);
                ctx->compilers.register_compiler(".c++", comp);
            }
        }}
    }
}

