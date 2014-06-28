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

#include <memory>
#include <queue>
#include <utility>
#include <unordered_map>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/bimap.hpp>

#include <reaver/exception.h>
#include <reaver/thread_pool.h>

#include "target.h"

namespace reaver
{
    namespace despayre { inline namespace _v1
    {
        class runner
        {
        public:
            runner() {}
            virtual ~runner() {}

            virtual bool operator()(const std::string & target, target_registry && reg) const
            {
                return (*this)(target,  reg);
            }

            virtual bool operator()(const std::string & target, target_registry & reg = default_target_registry()) const = 0;
        };

        class cycle_detected : public exception
        {
        public:
            cycle_detected() : exception{ logger::error }
            {
                *this << "a dependency cycle has been detected.";
            }
        };

        class thread_runner : public runner
        {
        public:
            thread_runner(std::size_t threads = 1) : _threads{ threads }
            {
            }

            virtual bool operator()(const std::string & target, target_registry & reg = default_target_registry()) const override
            {
                using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
                using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

                Graph graph{};
                boost::bimap<std::string, Vertex> vertexes;

                std::function<void (const std::string &)> add = [&](const std::string & name)
                {
                    auto & target = reg[name];

                    if (vertexes.left.find(name) == vertexes.left.end())
                    {
                        vertexes.left.insert(std::make_pair(name, boost::add_vertex(graph)));
                    }

                    for (auto && dependency : target.dependencies())
                    {
                        if (vertexes.left.find(dependency) == vertexes.left.end())
                        {
                            vertexes.left.insert(std::make_pair(dependency, boost::add_vertex(graph)));
                        }

                        if (boost::edge(vertexes.left.at(dependency), vertexes.left.at(name), graph).second)
                        {
                            throw cycle_detected{};
                        }

                        boost::add_edge(vertexes.left.at(dependency), vertexes.left.at(name), graph);
                        add(dependency);
                    }
                };

                add(target);

                std::deque<Vertex> sorted;
                boost::topological_sort(graph, std::front_inserter(sorted));
                std::atomic<bool> passed{ true };

                {
                    reaver::thread_pool pool{ _threads };

                    for (auto && target : sorted)
                    {
                        if (!passed)
                        {
                            return false;
                        }

                        try
                        {
                            pool.push([&, target = std::ref(reg[vertexes.right.at(target)])]()
                            {
                                for (auto && name : target.get().dependencies())
                                {
                                    auto && dep = reg[name];

                                    while (!dep.built())
                                    {
                                        if (dep.failed())
                                        {
                                            target.get().fail();
                                            return;
                                        }

                                        dep.wait_on();
                                    }
                                }

                                if (!target.get().build())
                                {
                                    passed = false;
                                    pool.abort();
                                }
                            });
                        }

                        catch (thread_pool_closed &)
                        {
                            return false;
                        }
                    }
                }

                return passed;
            }

        protected:
            std::size_t _threads;
        };

        class invalid_default_runner_initialization : public exception
        {
        public:
            invalid_default_runner_initialization() : exception{ logger::crash }
            {
                *this << "attempted to initialize Despayre's default runner with a null value.";
            }
        };

        inline runner & default_runner(std::unique_ptr<runner> new_default = nullptr)
        {
            static std::unique_ptr<runner> default_runner = [&]()
            {
                if (new_default)
                {
                    return std::move(new_default);
                }

                throw invalid_default_runner_initialization{};
            }();

            if (new_default)
            {
                default_runner = std::move(new_default);
            }

            return *default_runner;
        }
    }}
}
