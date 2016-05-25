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

#include <reaver/prelude/functor.h>
#include <reaver/filesystem.h>

#include "../semantics/string.h"
#include "../semantics/target.h"
#include "compiler.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class file;

        inline auto & file_registry()
        {
            static std::vector<std::shared_ptr<file>> files;
            return files;
        }

        inline std::shared_ptr<target> maybe_get_generated_file_target(context_ptr ctx, boost::filesystem::path path)
        {
            auto absolute = boost::filesystem::absolute(path);
            if (ctx->generated_files.find(absolute) != ctx->generated_files.end())
            {
                return ctx->generated_files.at(absolute);
            }
            return {};
        }

        class file : public target
        {
        public:
            file(boost::filesystem::path path) : target{ get_type_identifier<file>() }
            {
                [&]{
                    if (path.is_absolute())
                    {
                        _path = std::move(path);
                        return;
                    }

                    auto relative = filesystem::make_relative(path);
                    if (relative.begin()->string() == "..")
                    {
                        _path = boost::filesystem::absolute(std::move(path));
                        return;
                    }

                    _path = std::move(relative);
                }();
            }

            virtual bool built(context_ptr ctx) override
            {
                auto outs = outputs(ctx);
                assert(outs.size());

                for (auto && output : outs)
                {
                    if (!boost::filesystem::exists(output))
                    {
                        return false;
                    }
                }

                auto ins = inputs(ctx);
                assert(ins.size());

                auto input_times = fmap(ins, [](auto && path) {
                    return boost::filesystem::last_write_time(path);
                });
                auto output_times = fmap(outs, [](auto && path) {
                    return boost::filesystem::last_write_time(path);
                });

                return *std::max_element(input_times.begin(), input_times.end()) < *std::min_element(output_times.begin(), output_times.end());
            }

            const boost::filesystem::path & value() const
            {
                return _path;
            }

            std::vector<boost::filesystem::path> inputs(context_ptr ctx) const
            {
                return ctx->compilers.get_compiler(_path)->inputs(ctx, _path);
            }

            std::vector<boost::filesystem::path> outputs(context_ptr ctx) const
            {
                return ctx->compilers.get_compiler(_path)->outputs(ctx, _path);
            }

            virtual const std::vector<std::shared_ptr<target>> & dependencies(context_ptr ctx) override
            {
                if (!_deps || ctx != _cached_context)
                {
                    auto maybe_deps = fmap(inputs(ctx), [&](boost::filesystem::path argument) {
                        return maybe_get_generated_file_target(ctx, std::move(argument));
                    });
                    // ...I need to finally write this dream ranges library of mine...
                    _deps = std::vector<std::shared_ptr<target>>{};
                    for (auto && maybe_dep : maybe_deps)
                    {
                        if (maybe_dep)
                        {
                            _deps->emplace_back(std::move(maybe_dep));
                        }
                    }
                    _cached_context = ctx;
                }
                return *_deps;
            }

            virtual void invalidate() override
            {
                _cached_context = nullptr;
            }

        protected:
            virtual void _build(context_ptr ctx) override
            {
                ctx->compilers.get_compiler(_path)->build(ctx, _path);
            }

        private:
            boost::filesystem::path _path;
            std::shared_ptr<compiler> _compiler;
            optional<std::vector<std::shared_ptr<target>>> _deps;
            context_ptr _cached_context;
        };

        inline std::shared_ptr<target> get_file_target(context_ptr ctx, boost::filesystem::path path)
        {
            auto absolute = boost::filesystem::absolute(path);
            auto & target = ctx->file_targets[std::move(absolute)];

            if (!target)
            {
                target = std::make_shared<file>(std::move(path));
            }

            return target;
        }

        class files : public target
        {
            files() : target{ get_type_identifier<files>() }
            {
                _add_addition(get_type_identifier<files>(), [](_op_arg lhs, _op_arg rhs) -> std::shared_ptr<variable> {
                    auto lhs_files = lhs->as<files>();
                    auto rhs_files = rhs->as<files>();

                    std::vector<boost::filesystem::path> result;
                    std::set_union(lhs_files->_args.begin(), lhs_files->_args.end(), rhs_files->_args.begin(), rhs_files->_args.end(), std::back_inserter(result));
                    return std::make_shared<files>(std::move(result));
                });

                _add_removal(get_type_identifier<files>(), [](_op_arg lhs, _op_arg rhs) -> std::shared_ptr<variable> {
                    auto lhs_files = lhs->as<files>();
                    auto rhs_files = rhs->as<files>();

                    std::vector<boost::filesystem::path> result;
                    std::set_difference(lhs_files->_args.begin(), lhs_files->_args.end(), rhs_files->_args.begin(), rhs_files->_args.end(), std::back_inserter(result));
                    return std::make_shared<files>(std::move(result));
                });
            }

        public:
            files(std::vector<std::shared_ptr<variable>> args) : files()
            {
                auto paths = fmap(args, [](std::shared_ptr<variable> arg) {
                    return boost::filesystem::path(utf8(arg->as<string>()->value()));
                });
                std::sort(paths.begin(), paths.end());
                std::unique_copy(paths.begin(), paths.end(), std::back_inserter(_args));
            }

            files(std::vector<boost::filesystem::path> paths) : files()
            {
                std::sort(paths.begin(), paths.end());
                std::unique_copy(std::make_move_iterator(paths.begin()), std::make_move_iterator(paths.end()), std::back_inserter(_args));
            }

            virtual const std::vector<std::shared_ptr<target>> & dependencies(context_ptr ctx) override
            {
                if (!_file_deps || ctx != _cached_context)
                {
                    _file_deps = fmap(_args, [&](boost::filesystem::path argument) {
                        return get_file_target(ctx, std::move(argument));
                    });
                    _cached_context = ctx;
                }
                return *_file_deps;
            }

            virtual void invalidate() override
            {
                _cached_context = nullptr;
            }

            virtual bool built(context_ptr ctx) override
            {
                for (auto && dep : dependencies(ctx))
                {
                    if (!dep->built(ctx))
                    {
                        return false;
                    }
                }
                return true;
            }

        protected:
            virtual void _build(context_ptr) override
            {
            }

        private:
            std::vector<boost::filesystem::path> _args;
            optional<std::vector<std::shared_ptr<target>>> _file_deps;
            context_ptr _cached_context;
        };

        struct glob_tag {};

        inline std::shared_ptr<variable> glob(std::vector<std::shared_ptr<variable>> arguments)
        {
            return std::make_shared<files>(filesystem::wildcard(utf8(arguments[0]->as<string>()->value())));
        }
    }}
}

