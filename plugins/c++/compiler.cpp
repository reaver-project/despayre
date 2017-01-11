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

#include <fstream>
#include <cstdlib>
#include <regex>

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

    const char * compiler_detector =
#include "compiler_detector.h"
    ;
}

void reaver::despayre::cxx::_v1::cxx_compiler::_detect_compiler()
{
    const char * env_cxx = std::getenv("CXX");
    if (!env_cxx)
    {
        env_cxx = "c++";
    }

    _compiler_path = env_cxx;

    if (!_compiler_path.is_absolute())
    {
        assert(_compiler_path == _compiler_path.filename());

        auto path_cstr = std::getenv("PATH");
        assert(path_cstr);

        auto env_path = std::string{ path_cstr };

        std::vector<std::string> paths;
        paths.reserve(std::count(env_path.begin(), env_path.end(), ':'));
        boost::algorithm::split(paths, env_path, boost::is_any_of(":"));

        for (auto && path : paths)
        {
            if (boost::filesystem::exists(path / _compiler_path))
            {
                _compiler_path = path / _compiler_path;
                break;
            }
        }
    }

    assert(boost::filesystem::exists(_compiler_path) && "error out nicely about not being able to find the specified compiler");

    logger::dlog() << " -- Found a C++ compiler: " << _compiler_path.string();

    std::vector<std::string> args = { "/bin/sh", "-c", "exec " + _compiler_path.string() + " -x c++ -E - <<'EOF'\n" + compiler_detector + "\nEOF" };

    using namespace boost::process::initializers;
    boost::process::pipe p = boost::process::create_pipe();

    int exit_code = 0;

    {
        boost::iostreams::file_descriptor_sink sink{ p.sink, boost::iostreams::close_handle };
        auto child = boost::process::execute(set_args(args), inherit_env(), bind_stdout(sink), close_stdin());
        auto exit_status = wait_for_exit(child);
        exit_code = WEXITSTATUS(exit_status);
    }

    if (!exit_code)
    {
        boost::iostreams::file_descriptor_source source{ p.source, boost::iostreams::close_handle };
        boost::iostreams::stream<boost::iostreams::file_descriptor_source> is(source);

        std::string buffer(std::istreambuf_iterator<char>(is.rdbuf()), std::istreambuf_iterator<char>());

        _parse_detection_output(std::move(buffer));
    }

    static std::unordered_map<vendor, const char *> compiler_names = {
        { vendor::gcc, "G++" },
        { vendor::clang, "Clang" },
        { vendor::unknown, "unknown" }
    };

    logger::dlog() << " -- C++ compiler identification: " << compiler_names.at(_vendor) << " " << _version;
}

void reaver::despayre::cxx::_v1::cxx_compiler::_parse_detection_output(std::string output)
{
    std::stringstream stream{ std::move(output) };
    std::string buffer;

    std::regex message_pattern{ R"(^despayre: ([^:]+): (.*)$)" };
    std::smatch match;

    std::unordered_map<std::string, std::string> messages;

    while (std::getline(stream, buffer))
    {
        if (std::regex_match(buffer, match, message_pattern))
        {
            if (match[1] == "error")
            {
                throw std::runtime_error{ match[2] };
            }

            auto it = messages.find(match[1]);
            if (it != messages.end())
            {
                assert(!"a duplicate message from the detector!");
            }

            messages.emplace(match[1], match[2]);
        }
    }

    static std::unordered_map<std::string, vendor> compiler_ids = {
        { "G++", vendor::gcc },
        { "Clang++", vendor::clang },
        { "unknown", vendor::unknown }
    };

    _vendor = compiler_ids.at(messages.at("compiler-id"));

    if (_vendor == vendor::unknown)
    {
        return;
    }

    _version = messages.at("compiler-version");
    _default_cxx_version = messages.at("c++");
    _is_strict_by_default = messages.at("compiler-strict") == "true";
}

std::vector<std::string> reaver::despayre::cxx::_v1::cxx_compiler::_build_command(context_ptr ctx, const boost::filesystem::path & path) const
{
    auto out = filesystem::make_relative(output_path(ctx, path));

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

    auto compiler_specific_flags = [&]() -> std::u32string {
        if (_vendor == vendor::unknown)
        {
            return U"";
        }

        std::unordered_map<vendor, const char32_t *> nss = {
            { vendor::gcc, U"gcc" },
            { vendor::clang, U"clang" }
        };

        try
        {
            return _arguments->get_property(nss.at(_vendor))->get_property(U"flags")->as<string>()->value();
        }
        catch (...)
        {
            return std::u32string{};
        }
    }();

    auto deps_flags = " -MD -MF " + dependencies_path(ctx, path).string() + " ";

    auto cxxflags = []() -> std::string {
        auto cxxflags_env = std::getenv("CXXFLAGS");
        if (!cxxflags_env)
        {
            return {};
        }
        return cxxflags_env;
    }();

    std::vector<std::string> args = { "/bin/sh", "-c",
        "exec " + _compiler_path.string() + " -c " + " -std=c++1z -o '"
            + out.string() + "' '" + path.string() + "' "
            + utf8(flags) + " " + utf8(compiler_specific_flags) + deps_flags + cxxflags };

    return args;
}

std::vector<boost::filesystem::path> reaver::despayre::cxx::_v1::cxx_compiler::inputs(context_ptr ctx, const boost::filesystem::path & path) const
{
    auto inputs = filesystem::all_symlinked_paths(_compiler_path);

    auto deps_path = dependencies_path(ctx, path);

    if (boost::filesystem::exists(deps_path))
    {
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

    inputs.push_back(path);
    return inputs;
}

std::vector<boost::filesystem::path> reaver::despayre::cxx::_v1::cxx_compiler::outputs(context_ptr ctx, const boost::filesystem::path & path) const
{
    // probably needs stuff; maybe not in this case
    return { output_path(ctx, path) };
}

bool reaver::despayre::cxx::_v1::cxx_compiler::needs_rebuild(context_ptr ctx, const boost::filesystem::path & path) const
{
    if (!boost::filesystem::exists(output_path(ctx, path).string() + ".command"))
    {
        return true;
    }

    auto args = _build_command(ctx, path);
    std::ifstream command_file{ output_path(ctx, path).string() + ".command" };
    std::string last_command{ std::istreambuf_iterator<char>{ command_file.rdbuf() }, std::istreambuf_iterator<char>{} };

    return args.back() != last_command;
}

void reaver::despayre::cxx::_v1::cxx_compiler::build(context_ptr ctx, const boost::filesystem::path & path) const
{
    auto out = filesystem::make_relative(output_path(ctx, path));

    logger::dlog() << "Building " << out.string() << " from " << path.string() << ".";

    boost::filesystem::create_directories(out.parent_path());

    auto args = _build_command(ctx, path);

    {
        std::ofstream command_file{ output_path(ctx, path).string() + ".command" };
        command_file << args.back();
    }

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

    if (exit_code)
    {
        // TODO :P
        throw 1;
    }
}

