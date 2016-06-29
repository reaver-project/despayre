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

#include <fstream>

#include <reaver/filesystem.h>

#include <boost/process.hpp>
#ifndef BOOST_POSIX_API
# error only posix for now, sorry
#endif
#include <sys/wait.h> // eww, but boost.process 0.5 still doesn't abstract WEXITSTATUS away
#include <boost/iostreams/stream.hpp>

#include "compiler.h"
#include "despayre/semantics/string.h"

using reaver::despayre::_v1::context_ptr;

namespace
{
    boost::filesystem::path output_path(context_ptr ctx, boost::filesystem::path path)
    {
        path += ".o";
        return ctx->output_directory / path;
    }

    boost::filesystem::path dependencies_path(context_ptr ctx, boost::filesystem::path path)
    {
        auto output = output_path(ctx, std::move(path));
        output += ".deps";
        return output;
    }
}

std::vector<boost::filesystem::path> reaver::despayre::cxx::_v1::cxx_compiler::inputs(context_ptr ctx, const boost::filesystem::path & path) const
{
    auto deps_path = dependencies_path(ctx, path);

    if (boost::filesystem::exists(deps_path))
    {
        std::vector<boost::filesystem::path> inputs;

        std::fstream file{ deps_path.string(), std::ios::in };
        std::string buffer{ std::istreambuf_iterator<char>{ file.rdbuf() }, {} };
        auto start = buffer.begin();
        auto end = buffer.end();

        while (*start++ != ':')
        {
        }

        auto current = start;
        ++current;
        assert(current != start);

        while (start != end)
        {
            while (*start == ' ' || *start == '\\' || *start == '\n')
            {
                ++start;

                if (start == end)
                {
                    break;
                }
            }

            if (start == end)
            {
                break;
            }

            auto current = start;
            bool escaped = false;

            while (current != end && ((*current != ' ' && *current != '\n') || escaped))
            {
                if (!escaped && *current == '\\')
                {
                    escaped = true;
                }
                else
                {
                    escaped = false;
                }

                ++current;
            }

            inputs.emplace_back(start, current);
            start = current;
        }

        return inputs;
    }

    return { path };
}

std::vector<boost::filesystem::path> reaver::despayre::cxx::_v1::cxx_compiler::outputs(context_ptr ctx, const boost::filesystem::path & path) const
{
    // probably needs stuff; maybe not in this case
    return { output_path(ctx, path) };
}

void reaver::despayre::cxx::_v1::cxx_compiler::build(context_ptr ctx, const boost::filesystem::path & path) const
{
    auto out = filesystem::make_relative(output_path(ctx, path));

    logger::dlog() << "Building " << out.string() << " from " << path.string() << ".";

    boost::filesystem::create_directories(out.parent_path());

    // need a better way to do this
    auto flags = [&]{
        try
        {
            return _arguments->get_property(U"flags")->as<string>()->value();
        }
        catch (...)
        {
            return std::u32string{};
        }
    }();

    auto deps_flags = " -MD -MF " + dependencies_path(ctx, path).string() + " ";

    std::vector<std::string> args = { "/bin/sh", "-c", "exec ${CXX} -c ${CXXFLAGS} -std=c++1z -o '" + out.string() + "' '" + path.string() + "' " + utf8(flags) + deps_flags };

    using namespace boost::process::initializers;
    boost::process::pipe p = boost::process::create_pipe();

    int exit_code = 0;

    {
        boost::iostreams::file_descriptor_sink sink{ p.sink, boost::iostreams::close_handle };
        auto child = boost::process::execute(set_args(args), inherit_env(), bind_stdout(sink), close_stdin());
        auto exit_status = wait_for_exit(child);
        exit_code = WEXITSTATUS(exit_status);
    }

    boost::iostreams::file_descriptor_source source{ p.source, boost::iostreams::close_handle };
    boost::iostreams::stream<boost::iostreams::file_descriptor_source> is(source);

    std::string buffer(std::istreambuf_iterator<char>(is.rdbuf()), std::istreambuf_iterator<char>());
    if (!buffer.empty())
    {
        logger::dlog() << buffer;
    }

    if (!exit_code)
    {
        // TODO :P
    }
}

