/**
 * Despayre License
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <condition_variable>

#include <reaver/logger.h>
#include <reaver/exception.h>

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class target;

        template<typename T>
        bool build(T && t)
        {
            return std::forward<T>(t).build();
        }

        template<typename T>
        std::string name(T && t)
        {
            return std::forward<T>(t).name();
        }

        template<typename T>
        std::vector<std::string> dependencies(T && t)
        {
            return std::forward<T>(t).dependencies();
        }

        template<typename T>
        bool built(T && t)
        {
            return std::forward<T>(t).built();
        }

        class target
        {
        public:
            target() {}

            template<typename T>
            target(T && value) : _ptr{ std::make_unique<_impl<typename std::remove_reference<T>::type>>(std::forward<T>(value)) }
            {
            }

            template<typename T>
            target & operator=(T && value)
            {
                _ptr = std::make_unique<_impl<typename std::remove_reference<T>::type>>(std::forward<T>(value));
                return *this;
            }

            target(const target & other) : _ptr{ other._ptr->clone() }
            {
            }

            target(target && other) : _ptr{ std::move(other._ptr) }
            {
            }

            ~target() {}

            std::string name() const
            {
                return _ptr->name();
            }

            bool build()
            {
                _failed = !_ptr->build();
//                _cv.notify_all();
                return !_failed;
            }

            void fail()
            {
                _failed = true;
//                _cv.notify_all();
            }

            bool built() const
            {
                return _ptr->built();
            }

            bool failed() const
            {
                return _failed;
            }

            std::vector<std::string> dependencies() const
            {
                return _ptr->dependencies();
            }

            void wait_on()
            {
//                std::unique_lock<std::mutex> lock{ _m };
//                _cv.wait(lock);
            }

        private:
            class _base
            {
            public:
                virtual ~_base() {}

                virtual bool build() = 0;

                virtual std::string name() const = 0;
                virtual std::vector<std::string> dependencies() const = 0;
                virtual bool built() const = 0;

                virtual std::unique_ptr<_base> clone() const = 0;
            };

            template<typename T>
            class _impl : public _base
            {
            public:
                _impl(T value) : _value{ std::move(value) }
                {
                }

                virtual ~_impl() {}

                virtual bool build() override
                {
                    using despayre::build;
                    return build(_value);
                }

                virtual std::string name() const override
                {
                    using despayre::name;
                    return name(_value);
                }

                virtual std::vector<std::string> dependencies() const override
                {
                    using despayre::dependencies;
                    return dependencies(_value);
                }

                virtual bool built() const override
                {
                    using despayre::built;
                    return built(_value);
                }

                virtual std::unique_ptr<_base> clone() const override
                {
                    return std::make_unique<_impl>(_value);
                }

            private:
                T _value;
            };

            std::unique_ptr<_base> _ptr;
            bool _failed = false;

            std::mutex _m;
            std::condition_variable _cv;
        };

        class duplicate_target : public exception
        {
        public:
            duplicate_target(const std::string & name) : exception(logger::error)
            {
                *this << "target `" << name <<  "` registered twice.";
            }
        };

        class unknown_target : public exception
        {
        public:
            unknown_target(const std::string & name) : exception(logger::error)
            {
                *this << "requested unknown target `" << name << "`.";
            }
        };

        class target_registry
        {
        public:
            void add(target t)
            {
                if (_targets.find(t.name()) != _targets.end())
                {
                    throw duplicate_target{ t.name() };
                }

                _targets.emplace(t.name(), std::move(t));
            }

            target & operator[](const std::string & name)
            {
                if (_targets.find(name) == _targets.end())
                {
                    throw unknown_target{ name };
                }

                return _targets[name];
            }

        private:
            std::map<std::string, target> _targets;
        };

        target_registry & default_target_registry()
        {
            static target_registry reg;

            return reg;
        }

        void add_target(target t)
        {
            default_target_registry().add(std::move(t));
        }
    }}
}
