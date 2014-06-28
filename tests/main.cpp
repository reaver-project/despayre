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

#include <reaver/mayfly/main.h>
#include <reaver/mayfly/teamcity.h>

#include <reaver/despayre.h>

struct foo
{
    bool _built = false;
    std::string _name;
    std::vector<std::string> deps;

    bool built() const
    {
        return _built;
    }

    bool build()
    {
        for (auto && dep : deps)
        {
            MAYFLY_REQUIRE(reaver::despayre::default_target_registry()[dep].built());
        }

        _built = true;
        return true;
    }

    std::string name() const
    {
        return _name;
    }

    std::vector<std::string> dependencies() const
    {
        return deps;
    }

    foo(std::string name, std::vector<std::string> deps = {}) : _name{ std::move(name) }, deps{ std::move(deps) }
    {
    }
};


MAYFLY_BEGIN_SUITE("basic tests");

MAYFLY_ADD_TESTCASE("dependencies with one thread", []
{
    reaver::despayre::add_target(foo{ "a", { "b", "c", "d" }});
    reaver::despayre::add_target(foo{ "b" });
    reaver::despayre::add_target(foo{ "c", { "b", "d" }});
    reaver::despayre::add_target(foo{ "d" });

    reaver::despayre::default_runner(std::make_unique<reaver::despayre::thread_runner>())("a");
});

MAYFLY_ADD_TESTCASE("dependencies with many threads", []
{
    reaver::despayre::add_target(foo{ "a", { "b", "c", "d" }});
    reaver::despayre::add_target(foo{ "b" });
    reaver::despayre::add_target(foo{ "c", { "b", "d" }});
    reaver::despayre::add_target(foo{ "d" });

    reaver::despayre::default_runner(std::make_unique<reaver::despayre::thread_runner>(8))("a");
});

MAYFLY_END_SUITE;
