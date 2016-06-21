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

#include <reaver/logger.h>
#include <reaver/filesystem.h>

#include <boost/process.hpp>
#ifndef BOOST_POSIX_API
# error only posix for now, sorry
#endif
#include <sys/wait.h> // eww, but boost.process 0.5 still doesn't abstract WEXITSTATUS away
#include <boost/iostreams/stream.hpp>

#include "linker.h"

void reaver::despayre::cxx::_v1::cxx_linker::_build(reaver::despayre::_v1::context_ptr ctx, const boost::filesystem::path & out, reaver::despayre::_v1::binary_type type, const std::vector<boost::filesystem::path> & inputs, const std::string & additional_flags) const
{
    std::string message;
    std::string flags;

    switch (type)
    {
        case binary_type::executable:
            message = "Building executable ";
            break;

        case binary_type::shared_library:
            message = "Building shared library ";
            flags = " -shared ";
            break;

        case binary_type::static_library:
            assert(!"static library not implemented yet");
    }

    logger::dlog() << message << out.string() << ".";

    std::string input_paths = " ";
    for (auto && input : inputs)
    {
        input_paths += "\"" + input.string() + "\" ";
    }

    boost::filesystem::create_directories(out.parent_path());
    std::vector<std::string> args = { "/bin/sh", "-c", "exec clang++ ${CXXFLAGS} -std=c++1z -o '" + out.string() + "' " + additional_flags + flags + input_paths };

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

