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

#include "despayre/semantics/semantics.h"
#include "despayre/semantics/delayed_variable.h"
#include "despayre/semantics/import.h"

#include "despayre/semantics/string.h"
#include "despayre/semantics/namespace.h"
#include "despayre/semantics/basic.h"
#include "despayre/runtime/files.h"
#include "despayre/runtime/executable.h"
#include "despayre/runtime/shared_library.h"

void reaver::despayre::_v1::register_builtins(reaver::despayre::_v1::semantic_context & ctx)
{
    create_type<string>(ctx, U"string", "<builtin>", nullptr);
    create_type<name_space>(ctx, U"namespace", "<builtin>", nullptr);

    create_type<print>(
        ctx,
        U"debug_print",
        "<builtin>",
        make_type_checking_constructor<print>({
            { get_type_identifier<string>(), 1 }
        })
    );

    create_type<aggregate>(
        ctx,
        U"aggregate",
        "<builtin>",
        &default_constructor<aggregate>
    );

    create_type<import_tag>(ctx, U"import", "<builtin>", generate_import(ctx));
    create_type<plugin_namespace>(ctx, U"plugin", "<builtin>", nullptr);

    create_type<file>(ctx, U"file", "<builtin>", nullptr);
    create_type<files>(
        ctx,
        U"files",
        "<builtin>",
        make_type_checking_constructor<files>({
            { get_type_identifier<string>(), {} }
        })
    );
    create_type<glob_tag>(ctx, U"glob", "<builtin>", &glob);

    create_type<executable>(
        ctx,
        U"executable",
        "<builtin>",
        make_type_checking_constructor<executable>({
            { get_type_identifier<files>(), {} },
            { get_type_identifier<shared_library>(), {} },
            { get_type_identifier<string>(), 1 }
        })
    );
    create_type<shared_library>(
        ctx,
        U"shared_library",
        "<builtin>",
        make_type_checking_constructor<shared_library>({
            { get_type_identifier<files>(), {} },
            { get_type_identifier<shared_library>(), {} },
            { get_type_identifier<string>(), 1 }
        })
    );
}

