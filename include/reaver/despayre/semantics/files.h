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

#include "set.h"
#include "string.h"
#include "target.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class file : public target_clone_wrapper<file>
        {
        public:
            file(boost::filesystem::path path) : target_clone_wrapper<file>{ get_type_identifier<file>() }
            {
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
            }

            virtual std::vector<std::shared_ptr<target>> dependencies() const override
            {
                return {};
            }

            virtual bool built() const override
            {
                return false;
            }

            const boost::filesystem::path & value() const
            {
                return _path;
            }

        protected:
            virtual void _build() override
            {
                logger::dlog() << "Building " << _path.string() << ".";
            }

        private:
            boost::filesystem::path _path;
        };

        std::shared_ptr<target> get_file_target(boost::filesystem::path path)
        {
            static std::unordered_map<
                boost::filesystem::path,
                std::shared_ptr<target>,
                boost::hash<boost::filesystem::path>
            > targets;

            auto absolute = boost::filesystem::absolute(path);
            auto & target = targets[std::move(absolute)];

            if (!target)
            {
                target = std::make_shared<file>(std::move(path));
            }

            return target;
        }

        class files : public clone_wrapper<files>
        {
        public:
            files(std::vector<std::shared_ptr<file>> args) : clone_wrapper<files>{ get_type_identifier<files>() }, _files{ std::move(args) }
            {
            }

            const auto & value() const
            {
                return _files;
            }

        private:
            std::vector<std::shared_ptr<file>> _files;
        };

        struct glob_tag {};

        std::shared_ptr<variable> files_constructor(std::vector<std::shared_ptr<variable>> arguments)
        {
            return std::make_shared<files>(fmap(arguments, [](std::shared_ptr<variable> argument) {
                auto & str = argument->as<string>()->value();
                return std::make_shared<file>(utf8(str));
            }));
        }

        std::shared_ptr<variable> glob(std::vector<std::shared_ptr<variable>> arguments)
        {
            return nullptr;
        }

        namespace _detail
        {
            static auto _register_file = reaver::once([]{ create_type<file>(U"file", "<builtin>", nullptr); });
            static auto _register_files = reaver::once([]{ create_type<files>(U"files", "<builtin>", &files_constructor); });
            static auto _register_glob = reaver::once([]{ create_type<glob_tag>(U"glob", "<builtin>", &glob); });
        }
    }}
}

