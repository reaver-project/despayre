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

#include <reaver/filesystem.h>

#include "filetype.h"

using reaver::despayre::_v1::context_ptr;

namespace
{
    boost::filesystem::path output_path(context_ptr ctx, boost::filesystem::path path)
    {
        path += ".o";
        return ctx->output_directory / path;
    }
}

std::vector<boost::filesystem::path> reaver::despayre::cxx::_v1::cxx_compiler::inputs(context_ptr, const boost::filesystem::path & path) const
{
    // needs stuff
    return { path };
}

std::vector<boost::filesystem::path> reaver::despayre::cxx::_v1::cxx_compiler::outputs(context_ptr ctx, const boost::filesystem::path & path) const
{
    // probably needs stuff; maybe not in this case
    return { output_path(ctx, path) };
}

void reaver::despayre::cxx::_v1::cxx_compiler::build(context_ptr ctx, const boost::filesystem::path & path) const
{
    logger::dlog() << "Building " << filesystem::make_relative(output_path(ctx, path)).string() << " from " << path.string() << ".";
}

